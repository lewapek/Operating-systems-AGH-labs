#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "config_header.h"

struct client
{
	int id;
	int queue;
	char name[MAX_CLIENTS_NAME_LENGTH];
	int state;
};

static void add_client(struct msgbuf *message);
static void remove_client(int client_no);
static void send_everyone(const char *message_text, int sender_no, time_t tm);
static void sigint_handler(int sig);
static void atexit_function();

static int server_queue;
static struct client clients[MAX_CLIENTS_NUM];
static int actual_clients_number = 0;

int main(int argc, char const *argv[])
{
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

	printf("\nserver:  server successfully created, waiting for clients...\n\n");

	struct msgbuf message;

	while(1)
	{
		usleep(500000);

		//clients registration
		if(msgrcv(server_queue, &message, MAX_CLIENTS_NAME_LENGTH, 0, IPC_NOWAIT) < 0)
		{
			if(errno == E2BIG)
			{
				printf("Client name is too long. Can't register.");
			}
			else if(errno != EAGAIN && errno != ENOMSG)//empty queue
			{
				ERROR_HANDLER("msgrcv");
			}
		}
		else
		{
			add_client(&message);
			char registration_info[strlen("server:   joined the chat\n\n") + strlen(message.mtext) + 1];
			sprintf(registration_info, "server:  %s joined the chat\n\n", message.mtext);
			send_everyone(registration_info, SERVER_SENDER_NO, (time_t) 0);	// SERVER_SENDER_NO - wiadomość od serwera
		}

		//messages read
		int i;
		time_t tm;
		for(i = 0; i < actual_clients_number; i++)
		{
			if(clients[i].state == CLIENT_PRESENT)
			{
				tm = time(NULL);
				if(tm == (time_t) -1)
				{
					ERROR_HANDLER("time");
				}

				if(msgrcv(clients[i].queue, &message, MAX_MESSAGE_SIZE, CLIENT_TO_SERVER_TYPE, IPC_NOWAIT) < 0)
				{
					if(errno == EAGAIN || errno == ENOMSG)//empty queue
					{
						continue;
					}
					else if(errno == EINVAL)
					{
						remove_client(i);
						char info[strlen("server:   left\n\n") + strlen(clients[i].name + 1)];
						sprintf(info, "server:  %s left\n\n", clients[i].name);
						send_everyone(info, SERVER_SENDER_NO, (time_t) 0); 
					}
					else
					{
						ERROR_HANDLER("msgrcv");
					}
				}
				else
				{
					if(strcmp(message.mtext, END_CHAT) != 0)
					{
						send_everyone(message.mtext, i, tm);
					}
				}
			}
		}
	}

	exit(EXIT_SUCCESS);
}

static void add_client(struct msgbuf *message)
{
	key_t tmp_key;
	clients[actual_clients_number].id = message->mtype;
	clients[actual_clients_number].state = CLIENT_PRESENT;
	strcpy(clients[actual_clients_number].name, message->mtext);
	
	if((tmp_key = ftok(".", message->mtype)) == -1)
	{
		ERROR_HANDLER("add_client");
	}
	
	if((clients[actual_clients_number].queue = msgget(tmp_key, 0600)) < 0)
	{
		ERROR_HANDLER("add_client");
	}
	
	++actual_clients_number;
}

static void remove_client(int client_no)
{
	clients[client_no].state = CLIENT_ABSENT;
}

static void send_everyone(const char *message_text, int sender_no, time_t tm)
{
	struct msgbuf message;
	message.mtype = SERVER_TO_CLIENT_TYPE;

	if(sender_no == SERVER_SENDER_NO)
	{
		strcpy(message.mtext, message_text);
	}
	else
	{
		sprintf(message.mtext, "%s@%s:\t%s\n\n", ctime(&tm), clients[sender_no].name, message_text);
	}
	
	printf("%s\n", message.mtext);
	int i;
	for(i = 0; i < actual_clients_number; ++i)
	{
		if(i != sender_no && clients[i].state == CLIENT_PRESENT)
		{
			if(msgsnd(clients[i].queue, &message, MAX_MESSAGE_SIZE, 0) < 0)
			{
				ERROR_HANDLER("send_everyone");
			}
		}
	}
}

static void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

static void atexit_function()
{
	if(msgctl(server_queue, IPC_RMID, (struct msqid_ds *)NULL) < 0)
	{
		ERROR_HANDLER("atexit_function");
	}
}

