#ifndef CHAT_CONFIG_H_
#define CHAT_CONFIG_H_

#define MAX_MESSAGE_SIZE 1024
#define MAX_CLIENTS_NUM 10
#define MAX_CLIENTS_NAME_LENGTH 100
#define SERVER_TO_CLIENT_TYPE 1
#define CLIENT_TO_SERVER_TYPE 2
#define CLIENT_PRESENT 1
#define CLIENT_ABSENT 2
#define SERVER_SENDER_NO -1
#define END_CHAT "quit"

struct msgbuf
{
	long mtype;
	char mtext[MAX_MESSAGE_SIZE];
};

#include <stdio.h>
#include <stdlib.h>
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

#endif /* CHAT_CONFIG_H_ */
