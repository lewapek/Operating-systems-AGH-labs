#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MAX_CLIENTS_NAME_LENGTH 100

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s client_name\n", name);
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

static void atexit_function();

mqd_t server_queue;
mqd_t client_queue;
char* queue_name;

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}
	else if(strlen(argv[1]) >= MAX_CLIENTS_NAME_LENGTH)
	{
		fprintf(stderr, "Too long client name. Max is %d\n", MAX_CLIENTS_NAME_LENGTH);
		USAGE;
		exit(EXIT_FAILURE);
	}

	atexit(atexit_function);

	queue_name = malloc(strlen("/_queue") + strlen(argv[1]) + 1);
	sprintf(queue_name, "/%s_queue", argv[1]);

	struct mq_attr attr;
	attr.mq_flags = O_NONBLOCK;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = 4096;
	
	client_queue = mq_open(queue_name, O_RDWR | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
	if(client_queue == -1)
	{
		ERROR_HANDLER("mq_open()");
	}
	
	server_queue = mq_open("/server_queue", O_RDWR);
	if(server_queue < 0)
	{
		ERROR_HANDLER("mq_open");
	}

	if(mq_send(server_queue, argv[1], strlen(argv[1]) + 1, 0) < 0)
	{
		ERROR_HANDLER("mq_send");
	}

	if(mq_close(server_queue) < 0)
	{
		ERROR_HANDLER("mq_close()");
	}

	printf("\nSuccesfully joined.\nTo quit the chat, simply type \"quit\"\n\n");

	//reading process
	pid_t writing_process = getpid();
	pid_t reading_process = fork();

	if(reading_process == 0)
	{
		int prio = -1;
		char buff[attr.mq_msgsize];
		while(kill(writing_process, 0) == 0)
		{	
			usleep(500000);

			if(mq_receive(client_queue, buff, attr.mq_msgsize, (unsigned int*) &prio) < 0)
			{
				if(errno == EAGAIN)
				{
					continue;
				}
				else
				{
					ERROR_HANDLER("mq_receive");
				}
			}

			if(prio == 0) //msg from server
			{
				printf("%s\n", buff);
			}
			else if(prio > 0) //msg from client
			{
				if(mq_send(client_queue, buff, strlen(buff), prio) < 0)
				{
					ERROR_HANDLER("mq_send()");
				}
			}
		}

		exit(EXIT_SUCCESS);
	}

	//writing process
	char buff[attr.mq_msgsize];
	do
	{
		char c;
		int i;
		for(i = 0; i < attr.mq_msgsize && (c = fgetc(stdin)) != '\n'; i++)
		{
			buff[i] = c;
		}
		buff[i] = '\0';

		printf("\n");

		mq_send(client_queue, buff, strlen(buff) + 1, 1);
	} while(strcmp(buff, "quit"));


	mq_unlink(queue_name);
	exit(EXIT_SUCCESS);
}

static void atexit_function()
{
	mq_unlink(queue_name);
	free(queue_name);
}
