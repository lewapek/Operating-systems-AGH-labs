#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define SIZE 5
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

void* thread_function(void* arg);
void pickup_fork(int philosopher_number, int fork_no);
void putdown_fork(int fork_no);
void show_table_state();

int IDs[SIZE];
pthread_t philosophers[SIZE];
pthread_mutex_t forks[SIZE];

int who_is_eating[SIZE];
int who_has_fork[SIZE];

int main(int argc, char const *argv[])
{
	pthread_mutexattr_t attr;

	if((errno = pthread_mutexattr_init(&attr)) > 0)
	{
		ERROR_HANDLER("pthread_mutexattr_init");
	}
	if((errno = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL)) > 0)
	{
		ERROR_HANDLER("pthread_mutexattr_settype");
	}

	int i;	
	for (i = 0; i < SIZE; ++i)
	{
		IDs[i] = i;
		who_is_eating[i] = 0;		// doesn't eat now
		who_has_fork[i] = -1;	// -1: no such fork
		if((errno = pthread_mutex_init(forks + i, &attr)) > 0)
		{
			ERROR_HANDLER("pthread_mutex_init");
		}
	}

	srand((unsigned int) time(NULL));
	for (i = 0; i < SIZE; ++i)
	{
		if((errno = pthread_create(philosophers + i, NULL, thread_function, IDs + i)) != 0)
		{
			ERROR_HANDLER("pthread_create");
		}
	}

	while(1)
	{
		show_table_state();
		usleep(100000);
	}

	for (i = 0; i < SIZE; ++i)
	{
		if((errno = pthread_join(philosophers[i], NULL)) != 0)
		{
			ERROR_HANDLER("pthread_join");
		}
	}

	exit(EXIT_SUCCESS);
}

void* thread_function(void* arg){
	int philosopher_number = *((int*) arg);
	int left_fork = philosopher_number;
	int right_fork = (philosopher_number + 1) % 5;

	while(1)
	{
		usleep(rand() % 100);

		if(philosopher_number % 2)
		{
			pickup_fork(philosopher_number, left_fork);
			pickup_fork(philosopher_number, right_fork);
		}							
		else
		{
			pickup_fork(philosopher_number, right_fork);
			pickup_fork(philosopher_number, left_fork);
		}

		who_is_eating[philosopher_number] = 1;
		usleep(rand() % 100);
		who_is_eating[philosopher_number] = 0;

		putdown_fork(left_fork);
		putdown_fork(right_fork);
	}

	return NULL;
}

void pickup_fork(int philosopher_number, int fork_no)
{
	if((errno = pthread_mutex_lock(forks + fork_no)) != 0)
	{
		ERROR_HANDLER("pthread_mutex_lock");
	}

	who_has_fork[fork_no] = philosopher_number;
}

void putdown_fork(int fork_no)
{
	who_has_fork[fork_no] = -1;	

	if((errno = pthread_mutex_unlock(forks + fork_no)) != 0)
	{
		ERROR_HANDLER("pthread_mutex_unlock");
	}
}

void show_table_state()
{
	printf("Philosophers:");
	int i;
	for(i = 0; i < SIZE; ++i)
	{
		if(who_is_eating[i])
		{
			printf(" #%d,", i);
		}
	}
	printf(" are currently eating\n");
	for(i = 0; i < SIZE; ++i)
	{
		if(who_has_fork[i] != -1)
		{
			printf(" #%d has fork %d,", who_has_fork[i], i);
		}
	}
	printf("\n\n");
}
