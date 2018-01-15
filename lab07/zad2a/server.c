#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s max_file_length\n", name);

static void sigint_handler(int sig);
static void atexit_function();

static int server_queue;
static int max_file_length;
static FILE* file;

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

	key_t server_key = ftok(".", 1);
	if(server_key == -1)
	{
		ERROR_HANDLER("ftok");
	}

	server_queue = msgget(server_key, IPC_CREAT | 0600);
	if(server_queue < 0)
	{
		ERROR_HANDLER("msgget");
	}

	printf("\nserver:  server created, waiting from clients\n\n");

	struct msgbuf message;

	file = fopen("messages.txt", "w+");
	if(file == NULL)
	{
		ERROR_HANDLER("fopen");
	}

	struct stat stats;
	while(1)
	{
		usleep(500000);

		if(stat("messages.txt", &stats) == -1)
		{
			ERROR_HANDLER("stat");
		}

		time_t tm = time(NULL);
		if(msgrcv(server_queue, &message, MAX_CLIENTS_NAME_LENGTH + RAND_STR_LENGTH + 8, 0, IPC_NOWAIT) < 0)
		{
			if(errno != EAGAIN && errno != ENOMSG)
			{
				ERROR_HANDLER("msgrcv");
			}
		}
		else
		{
			if((strlen(message.mtext) + strlen(ctime(&tm)) + 2 + stats.st_size) > max_file_length)
			{
				if(kill((pid_t) message.mtype, SIGUSR2) == -1)
				{
					ERROR_HANDLER("kill");
				}
				printf("file is full, can't save the message\n\n");
			}
			else
			{
				if(kill((pid_t) message.mtype, SIGUSR1) == -1)
				{
					ERROR_HANDLER("kill");
				}
				printf("%s%s\n\n", ctime(&tm), message.mtext);
				
				if(fprintf(file, "%s%s\n\n", ctime(&tm), message.mtext) < 0)
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

static void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

static void atexit_function()
{
	fclose(file);
	
	if(msgctl(server_queue, IPC_RMID, (struct msqid_ds *) NULL) < 0)
	{
		ERROR_HANDLER("atexit_function");
	}
}
