#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s max_file_length\n", name);

void sigint_handler(int signo);
void atexit_function();

mqd_t server_q;
int max_file_length;
FILE* file;

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number ofarguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	max_file_length = atoi(argv[1]);

	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	struct mq_attr attr;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = RAND_STR_LENGTH + MAX_CLIENTS_NAME_LENGTH + 8;
	attr.mq_flags = O_NONBLOCK;

	server_q = mq_open("/server_queue", O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &attr);
	if(server_q < 0)
	{
		ERROR_HANDLER("mq_open()");
	}

	printf("\nserver:  server created, waiting for messages from clients\n\n");

	char message[RAND_STR_LENGTH + MAX_CLIENTS_NAME_LENGTH + 8];

	file = fopen("messages.txt", "w+");
	if(file == NULL)
	{
		ERROR_HANDLER("fopen");
	}

	struct stat stats;
	unsigned int client_pid;
	while(1)
	{
		usleep(500000);

		if(stat("messages.txt", &stats) == -1)
		{
			ERROR_HANDLER("stat");
		}

		time_t tm = time(NULL);
		if(mq_receive(server_q, message, RAND_STR_LENGTH + MAX_CLIENTS_NAME_LENGTH + 8, &client_pid) < 0)
		{
			if(errno != EAGAIN)
			{
				ERROR_HANDLER("mq_receive");
			}
		}
		else
		{
			if((strlen(message) + strlen(ctime(&tm)) + 2 + stats.st_size) > max_file_length)
			{
				if(kill(client_pid, SIGUSR2) == -1)
				{
					ERROR_HANDLER("kill");
				}

				printf("file is full, can't save the message\n\n");
			}
			else
			{
				if(kill(client_pid, SIGUSR1) == -1)
				{
					ERROR_HANDLER("kill");
				}

				printf("%s%s\n\n", ctime(&tm), message);
				if(fprintf(file, "%s%s\n\n", ctime(&tm), message) < 0)
				{
					ERROR_HANDLER("fprintf");
				}

				if(fflush(file) == EOF)
				{
					ERROR_HANDLER("fflush");
				}
			}
		}
	}

	exit(EXIT_SUCCESS);
}

void sigint_handler(int signo)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	fclose(file);
	if(mq_unlink("/server_queue") < 0)
	{
		ERROR_HANDLER("atexit_function");
	}
}
