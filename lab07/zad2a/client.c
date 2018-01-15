#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s client_name\n", name)

void signal_handler(int sig);

#define SUCCESSFULLY_SAVED 1
#define FILE_IS_FULL 2
int answer_from_serwer = 0;

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

	pid_t pid = getpid();

	signal(SIGUSR1, signal_handler);
	signal(SIGUSR2, signal_handler);

	key_t server_key = ftok(".", 1);
	if(server_key == -1)
	{
		ERROR_HANDLER("ftok");
	}

	int server_queue = msgget(server_key, 0200);
	if(server_queue < 0)
	{
		ERROR_HANDLER("msgget");
	}

	printf("\nSuccesfully joined.\n\n");

	struct msgbuf message;
	message.mtype = pid;
	sprintf(message.mtext, "%s;%d;", argv[1], pid);

	int index = strlen(message.mtext);

	srand(time(NULL));
	int i;
	for(i = 0;i < RAND_STR_LENGTH; i++)
	{
		message.mtext[index++] = 'a' + rand() % 26;
	}
	message.mtext[index] = '\0';

	printf("%s\n", message.mtext);
	if(msgsnd(server_queue, &message, MAX_CLIENTS_NAME_LENGTH + RAND_STR_LENGTH + 8, 0) < 0)
	{
		ERROR_HANDLER("msgsnd");
	}

	while(answer_from_serwer != SUCCESSFULLY_SAVED && answer_from_serwer != FILE_IS_FULL)
	{
		pause();
	}

	if(answer_from_serwer == FILE_IS_FULL)
	{
		printf("\nserver:  Can't save your message, file is full\n\n");
	}
	else
	{
		printf("\nserver:  Your message has been successfully saved in messages.txt\n\n");
	}

	exit(EXIT_SUCCESS);
}

void signal_handler(int sig)
{
	if(sig == SIGUSR1)
	{
		answer_from_serwer = SUCCESSFULLY_SAVED;
	}
	else if(sig == SIGUSR2)
	{
		answer_from_serwer = FILE_IS_FULL;
	}
}
