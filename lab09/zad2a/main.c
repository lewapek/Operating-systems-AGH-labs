// #define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>

struct data_t
{
	pthread_t * threads;
	int threads_quantity;
	int buffer_size;
	int fd;
	int position;
	char *pattern;
	sigset_t mask;
};

struct thread_data_t
{
	struct data_t * data;
	int position;
};

pthread_mutex_t mutex_find = PTHREAD_MUTEX_INITIALIZER; //mutex do zabezpieczania lseek, read
pthread_mutex_t mutex_cancel = PTHREAD_MUTEX_INITIALIZER; //mutex do zabezpieczania anulowania

void after_found_function(int position, struct data_t * data)//function to call after pattern is found
{
 	 printf("Thread %ld found pattern at position %d\n", pthread_self(), position);
#ifndef DETACH/*in versions 1. and 2. threads are canceled*/
	pthread_mutex_lock(&mutex_cancel); 	 
 	
 	int i;
	for(i=0; i<data->threads_quantity; i++)
 	{
 		while(data->threads[i] != 0)
	 	{
	 		continue;
	 	} 
 	 	if(!pthread_equal(data->threads[i], pthread_self()))
 	 	{
 	 		pthread_cancel(data->threads[i]);//cancel all except self
#ifdef SYNCHR
			printf("Thread %ld synchronous closed.\n",data->threads[i]);
#else
 	 		printf("Thread %ld asynchronous closed.\n",data->threads[i]);
#endif
		}
 	}
#ifdef SYNCHR
	data->threads_quantity = 0;//security for noone else try to cancel
#endif
 	pthread_mutex_unlock(&mutex_cancel); 
 	pthread_exit(NULL);
#endif
}
 
void thread_finish_info(void *data)
{
	printf("Thread tid %ld finishing.\n", pthread_self());
#ifdef DETACH //in case of version 3. there is a need to control running threads number to let main clean and finish
	struct data_t * d = data;
	pthread_mutex_lock(&mutex_cancel);
  
	d->threads_quantity--;
	pthread_mutex_unlock(&mutex_cancel);
#endif
} 
 
void finish_thread(void * data)
{ 	 
	char *buffer = data;
 	free(buffer);
}
 
void *find_pattern(void * arg)
{
	struct thread_data_t *d = arg;
	int err = pthread_sigmask(SIG_SETMASK, &(d->data->mask), NULL);
	if(err != 0)
	{
		fprintf(stderr, "pthread_sigmask error");
		pthread_exit(NULL);
	}

	pthread_cleanup_push(thread_finish_info, d->data);//thread_finish_info will be called at finish
	sleep(2);

#ifndef DETACH/*in case ov version 3. we care about cancelling options*/
#ifdef SYNCHR/*synchronous cancelling*/
  	err = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  	if(err != 0)
  	{
		fprintf(stderr,"Synchronous thread cancel error: %s\n", strerror(err));
  		pthread_exit(NULL);
  	}
  	
  	//canceling option block until interesting moment
  	err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  	if(err != 0)
  	{
  		fprintf(stderr,"Deactivation cancel thread error %s\n", strerror(err));
  		pthread_exit(NULL);
  	}
#else 
	//synchronous cancel
	err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	if(err!=0)
	{
		fprintf(stderr,"Asynchronous thread cancel error: %s\n",strerror(err));
  		pthread_exit(NULL);
  	}
#endif
#endif
	int fd=d->data->fd;
	int buffer_size = d->data->buffer_size;
	char *pattern = d->data->pattern;
	int pattern_length = strlen(pattern);

#ifdef DETACH
	pthread_mutex_lock(&mutex_cancel);
	int threads_quantity = d->data->threads_quantity;
	pthread_mutex_unlock(&mutex_cancel);
#else
	int threads_quantity = d->data->threads_quantity; 
#endif

	char *buffer=malloc(buffer_size*sizeof(char));
	int read_check;

	pthread_cleanup_push(finish_thread, buffer);
	pthread_mutex_lock(&mutex_find);

	d->position=lseek(fd,d->position*buffer_size,SEEK_SET);
	while(true)
	{
 		read_check = read(fd, buffer, buffer_size);
 		pthread_mutex_unlock(&mutex_find);
 		if(read_check < 0)
 		{
 	  		perror("read error");
 	 		pthread_exit(NULL);
 		}
 	
 		if(read_check == 0)
 		{
 			pthread_exit(NULL);
 		}

 		int i, k;
 		for(k=0; k<read_check; ++k)
 		{ 
 			//sign after sign compare
 			for(i = k; i < read_check && i - k < pattern_length && buffer[i] == pattern[i - k]; ++i)
 			{}
 			if(i - k == pattern_length)
 			{
 				after_found_function(d->position+k,d->data);
 			}
 			else if(i==read_check)
 			{
	 			int how_many_left = pattern_length - i + k;
	 			char *buf2 = malloc(sizeof(char) * how_many_left);
 
 				pthread_cleanup_push(finish_thread, buf2);
	 			pthread_mutex_lock(&mutex_find);
	 			lseek(fd, d->position+read_check, SEEK_SET);	
 				read_check = read(fd,buf2,how_many_left);
	 			pthread_mutex_unlock(&mutex_find);
	
	 			if(read_check < 0)
	 			{
	 				perror("read error");
	 				pthread_exit(NULL);
	 			}
	 			else if(!read_check)
	 			{
	 				pthread_exit(NULL);
	 			}
	 			else
	 			{
	 				int j;
	 				for(j=0; j<read_check && buf2[j]==pattern[j+i-k]; j++)
	 				{}
	 				if(j == read_check)
 					{
 						after_found_function(d->position+k,d->data);
 					}
	 			}
 				pthread_cleanup_pop(1); //Usuwanie buf2 (jesli wczesniej nie nastapilo)	
 			}
 		}
#ifdef SYNCHR
		//Odlokowujemy na chwile anulowanie w trybie 2
 		err = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        if(err != 0)
        {
  			fprintf(stderr, "thread cancel error %s\n", strerror(err));
  			pthread_exit(NULL);
  		};
  	
		pthread_testcancel(); //Sprawdzamy czy ktos nas nie anulowal
	
		//Przywracamy blokade anulowania
		err = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if(err != 0)
		{
  			fprintf(stderr,"Deactivate thread cancel error %s\n", strerror(err));
  			pthread_exit(NULL);
  		};
#endif
 		pthread_mutex_lock(&mutex_find); //Ustawiamy kolejna pozycje
 		d->position = lseek(fd, d->position + threads_quantity * buffer_size, SEEK_SET);
	}
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
}

void signal_handler(int sig)
{
	fprintf(stderr, "Signal received, pid: %d, tid: %ld\n", getpid(), pthread_self());
}

int main(int argc, char *argv[])
{
	char opt;
	int target = 1;
	int mask = -1;
	int action = 0;
	int sig = -1;
	char *usage = "Usage:\n\t"
				  "program_name [-t process|thread] [-m process|thread] [-a] "
				  "[-s SIGUSR1|SIGTERM|SIGKILL|SIGSTOP]"
				  "threads_number file buffer_length pattern\n"
				  "Options:\n"
				  "\t-t: where signal is send\n"
				  "\t-m: mast thread or process\n"
				  "\t-a: set soecific flag to handle signal for thread\n"
				  "\t-s: signal\n";

	while((opt = getopt(argc, argv, "t:m:as:"))!= -1)
	{
		if(opt == 't')
		{
			if(strcmp(optarg, "process") == 0)
			{
				target = 0;
			}
			else if(strcmp(optarg, "thread") == 0)
			{
				target = 1;
			}
			else
			{
				fprintf(stderr, "%s\n", usage);
				exit(EXIT_FAILURE);
			}
		}
		else if(opt == 'm')
		{
			if(strcmp(optarg, "process") == 0)
			{
				mask = 0;
			}
			else if(strcmp(optarg, "thread") == 0)
			{
				mask = 1;
			}
			else
			{
				fprintf(stderr, "%s\n", usage);
				exit(EXIT_FAILURE);
			}
		}
		else if(opt == 'a')
		{
			action = 1;
		}
		else if(opt == 's')
		{
			if(strcmp(optarg, "SIGUSR1") == 0)
			{
				sig = SIGUSR1;
			}
 	 	 	else if(strcmp(optarg, "SIGTERM") == 0)
 	 	 	{
 	 	 		sig = SIGTERM;
 	 	 	}
 	 	 	else if(strcmp(optarg, "SIGKILL") == 0)
 	 	 	{
 	 	 		sig = SIGKILL;
 	 	 	} 
 	 	 	else if(strcmp(optarg, "SIGSTOP") == 0)
 	 	 	{
 	 	 		sig = SIGSTOP;
 	 	 	}
 	 	 	else
 	 	 	{
 	 	 		fprintf(stderr, "Inappropriate argument.\n");
 	 	 	 	fprintf(stderr, "%s\n", usage);
 	 	 	 	exit(EXIT_FAILURE);
 	 	 	}
		}
	}
	if(argc - optind != 4)
	{
		fprintf(stderr,"%s\n", usage);
		exit(EXIT_FAILURE);
	}
  
  	struct data_t data;  
  	data.threads_quantity = atoi(argv[optind]);
  	data.buffer_size = atoi(argv[optind + 2]);
  	data.pattern=argv[optind + 3];
  	sigemptyset(&(data.mask));
  
  	if(sig != -1)
  	{
  		if(mask != -1)
  		{
  			sigset_t block;
  			sigemptyset(&block);
  			sigaddset(&block, sig);

  			if(mask == 0)
  			{
  				pthread_sigmask(SIG_SETMASK, &block, NULL); 
  			}
  			else
  			{
  				data.mask = block;
  			}
  		}

  		if(action != 0)
  		{
  			signal(sig, signal_handler);
  		}
  	}

	if((data.fd = open(argv[optind + 1], O_RDONLY)) < 0)
	{
  		perror("Error opening file");
  		exit(EXIT_FAILURE);
  	}
  
  	data.threads = malloc(data.threads_quantity * sizeof(pthread_t));
	struct thread_data_t *thread_data = malloc(data.threads_quantity * sizeof(struct thread_data_t));
  
 	int i,err;
  	//prevent from cancel non-existing thread
  	for(i=0; i<data.threads_quantity; i++)
	{
		data.threads[i]=0;
	}
  
  	for(i=0; i<data.threads_quantity; i++)
  	{
	  	thread_data[i].data=&data;
	  	thread_data[i].position = i;
	  	//thread create
	  	if((err = pthread_create(&(data.threads[i]), NULL, find_pattern, &(thread_data[i]))))
	  	{
	  		fprintf(stderr,"thread create error: %s\n",strerror(err));
	  		exit(EXIT_FAILURE);
	  	}
#ifdef DETACH
		if((err = pthread_detach(data.threads[i])) != 0)
		{
			fprintf(stderr,"thread datech error: %s\n",strerror(err));
			exit(EXIT_FAILURE);
		}
#endif
	}

	sleep(1);
	srand(time(NULL));
	if(sig != -1 && target != -1)
	{
		if(target == 0)
		{
			kill(getpid(), sig);
		}
		else
		{
			int index = rand() % data.threads_quantity;
			pthread_kill(data.threads[index], sig);
		}
	}	
#ifdef DETACH //W trybie 3 musimy poczekac az wszystkie threads skoncza prace
	while(true)
	{
    	pthread_mutex_lock(&mutex_cancel); //data.threads_quantity to zmienna wspoldzielona!
    
	    if(!data.threads_quantity)
    	{
    		break;
    	}
	    
	    pthread_mutex_unlock(&mutex_cancel);
    	sleep(1); 
	}
#else //W trybie 1 i 2 po prostu czekamy na threads przez przylaczenie
	for(i=0; i<data.threads_quantity; i++)
	{
	  	if((err = pthread_join(data.threads[i], NULL)))
	  	{
	  		fprintf(stderr,"thread join error: %s\n",strerror(err));
	  		exit(EXIT_FAILURE);
	  	}
  	}
#endif
	close(data.fd);
	free(thread_data);
	free(data.threads);
  
	return 0;  
}  
