#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "config_header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s client_name\n", name);

pid_t pid;
int server_queue;
int client_queue;

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

	pid = getpid();

	key_t client_key = ftok(".", pid);
	if(client_key == -1)
	{
		ERROR_HANDLER("ftok");
	}
	
	client_queue = msgget(client_key, IPC_CREAT | 0600);
	if(client_queue < 0)
	{
		ERROR_HANDLER("msgget");
	}
	
	key_t server_key = ftok(".", 1);
	if(server_key == -1)
	{
		ERROR_HANDLER("ftok");
	}
	
	server_queue = msgget(server_key, 0200);	// otwieranie kolejki serwera
	if(server_queue < 0)
	{
		ERROR_HANDLER("msgget");
	}
	
	printf("\nSuccesfully joined chat server.\nTo exit, type \"%s\"\n\n", END_CHAT);

	struct msgbuf message;
	message.mtype = pid;
	strcpy(message.mtext, argv[1]);

	if(msgsnd(server_queue, &message, MAX_CLIENTS_NAME_LENGTH, 0) < 0)
	{
		ERROR_HANDLER("msgsnd");
	}
	
	//reading process
	pid_t writing_process = getpid();
	pid_t reading_process = fork();

	if(reading_process == 0)
	{
		while(kill(writing_process, 0) == 0)
		{
			usleep(500000);

			if(msgrcv(client_queue, &message, MAX_MESSAGE_SIZE, SERVER_TO_CLIENT_TYPE, IPC_NOWAIT) < 0)
			{
				if(errno == EAGAIN || errno == ENOMSG)
				{
					continue;
				}
				else
				{
					ERROR_HANDLER("msgrcv");
				}
			}

			printf("%s", message.mtext);
		}

		if(msgctl(client_queue, IPC_RMID, (struct msqid_ds *)NULL) < 0)
		{
			ERROR_HANDLER("msgctl");
		}

		exit(EXIT_SUCCESS);
	}

	//reading process
	message.mtype = CLIENT_TO_SERVER_TYPE;
	do
	{
		char c;
		int i;
	
		for(i = 0; i < MAX_MESSAGE_SIZE && (c = fgetc(stdin)) != '\n'; ++i)
		{
			message.mtext[i] = c;
		}
		message.mtext[i] = (char) 0;

		printf("\n");

		if(msgsnd(client_queue, &message, MAX_MESSAGE_SIZE, 0) < 0)
		{
			ERROR_HANDLER("msgsnd");
		}
	} while(strcmp(message.mtext, END_CHAT));


	exit(EXIT_SUCCESS);
}
