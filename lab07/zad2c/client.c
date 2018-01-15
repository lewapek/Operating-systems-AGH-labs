#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <mqueue.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t %s client_name\n", name)

void signal_handler(int sig);

#define SUCCESSFULLY_SAVED 1
#define FILE_IS_FULL 2
int answer_from_serwer;

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

	if(signal(SIGUSR1, signal_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	if(signal(SIGUSR2, signal_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	mqd_t server_q = mq_open("/server_queue", O_WRONLY);
	if(server_q == -1)
	{
		ERROR_HANDLER("mq_open");
	}

	printf("\nSuccesfully connected to chat server.\n\n");

	char message[RAND_STR_LENGTH + MAX_CLIENTS_NAME_LENGTH + 8];

	sprintf(message, "%s;%d;", argv[1], pid);

	int index = strlen(message);
	int i;
	srand(time(NULL));
	for(i = 0;i < RAND_STR_LENGTH; ++i)
	{
		message[index++] = 'a' + rand() % 26;
	}
	message[index] = '\0';

	printf("%s\n", message);

	sigset_t new_set;
	sigemptyset(&new_set);
	sigaddset(&new_set, SIGUSR1);
	sigaddset(&new_set, SIGUSR2);

	if(sigprocmask(SIG_BLOCK, &new_set, NULL) == -1)
	{
		ERROR_HANDLER("sigprocmask");
	}

	if(mq_send(server_q, message, strlen(message) + 1, (unsigned int) pid) < 0)
	{
		ERROR_HANDLER("mq_send");
	}

	answer_from_serwer = 0;
	if(sigprocmask(SIG_UNBLOCK, &new_set, NULL) == -1)
	{
		ERROR_HANDLER("sigprocmask");
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
