#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define MAX_CLIENTS 5
#define MAX_CLIENT_NAME 128
#define SERVER_QUEUE "/server_queue"

#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

void add_user_handler(int sig);
void receive_msg_handler(int sig, siginfo_t *info, void* context);
void sigint_handler(int sig);
void atexit_function();

struct client
{
	mqd_t queue;
	char name[MAX_CLIENT_NAME];
	struct sigevent sigev;
};

struct sigevent sigev;
mqd_t server_q;
struct client clients_list[MAX_CLIENTS];
int actual_clients_number = 0;

int main(int argc, char const *argv[])
{
	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 50;
	attr.mq_msgsize = 100;
	
	server_q = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &attr);
	if(server_q == -1)
	{
		ERROR_HANDLER("mq_open");
	}
	
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGUSR1;
	sigset_t usr1_set;
	
	if(sigemptyset(&usr1_set) < 0)
	{
		ERROR_HANDLER("sigemptyset");
	}

	if(sigaddset(&usr1_set, SIGUSR1) < 0)
	{
		ERROR_HANDLER("sigaddset");
	}

	struct sigaction usr1_act;
	usr1_act.sa_handler = add_user_handler;
	usr1_act.sa_mask = usr1_set;

	if(sigaction(SIGUSR1, &usr1_act, NULL) == -1)
	{
		ERROR_HANDLER("sigaction");
	}

	if(mq_notify(server_q, &sigev) == -1)
	{
		ERROR_HANDLER("mq_notify");
	}

	sigset_t usr2_set;
	if(sigemptyset(&usr2_set) < 0)
	{
		ERROR_HANDLER("sigemptyset");
	}

	if(sigaddset(&usr2_set, SIGUSR2) < 0)
	{
		ERROR_HANDLER("sigaddset");
	}

	struct sigaction usr2_act;
	usr2_act.sa_sigaction = receive_msg_handler;
	usr2_act.sa_mask = usr2_set;
	usr2_act.sa_flags = SA_SIGINFO;

	if(sigaction(SIGUSR2, &usr2_act, NULL) < 0)
	{
		ERROR_HANDLER("sigaction");
	}

	printf("\nServer created successfully.\nTo finish, type CTRL^C.\n\n");

	while(1)
	{
		pause();
	}

	mq_unlink(SERVER_QUEUE);
	
	exit(EXIT_SUCCESS);
}

void add_user_handler(int sig)
{
	if(actual_clients_number >= MAX_CLIENTS)
	{
		printf("server:\ttoo many users");
		return;
	}

	if(mq_receive(server_q, clients_list[actual_clients_number].name, 100, NULL) == -1)//msg received
	{
		ERROR_HANDLER("add_user_handler");
	}

	char client_q_name[strlen("/_queue") + strlen(clients_list[actual_clients_number].name) + 1];
	if(sprintf(client_q_name, "/%s_queue", clients_list[actual_clients_number].name) < 0)//client queue name
	{
		ERROR_HANDLER("add_user_handler");
	}

	clients_list[actual_clients_number].queue = mq_open(client_q_name, O_RDWR);//putting queue in queues array
	clients_list[actual_clients_number].sigev.sigev_notify = SIGEV_SIGNAL;
	clients_list[actual_clients_number].sigev.sigev_signo = SIGUSR2;
	
	union sigval val;
	val.sival_int = actual_clients_number;
	clients_list[actual_clients_number].sigev.sigev_value = val;
	
	if(mq_notify(clients_list[actual_clients_number].queue, &clients_list[actual_clients_number].sigev) < 0)//register for notifications from user's queue
	{
		ERROR_HANDLER("add_user_handler");
	}

	printf("server:  %s has joined the chat\n\n", clients_list[actual_clients_number].name);
	++actual_clients_number;

	if(mq_notify(server_q, &sigev) == -1)//reregister
	{
		ERROR_HANDLER("add_user_handler");
	}
}

void receive_msg_handler(int sig, siginfo_t *info, void* context)
{
	int client_no = info->si_value.sival_int;
	int prio = -1, need_to_notify = 1;
	struct mq_attr attr;

	if(mq_getattr(clients_list[client_no].queue, &attr) < 0)
	{
		ERROR_HANDLER("receive_msg_handler");
	}

	char buff[attr.mq_msgsize];
	if(mq_receive(clients_list[client_no].queue, buff, attr.mq_msgsize, (unsigned int *)&prio) < 0)//msg receive
	{
		ERROR_HANDLER("receive_msg_handler");
	}

	if(prio == 0)//msg from server to client ==> back to queue
	{
		if(mq_send(clients_list[client_no].queue, buff, attr.mq_msgsize, (unsigned int)0) < 0)
		{
			ERROR_HANDLER("receive_msg_handler");
		}
	}
	else if(prio > 0)//msg from client
	{
		time_t msg_time = time(NULL);
		if(msg_time == (time_t) -1)
		{
			ERROR_HANDLER("receive_msg_handler");
		}
		char message[attr.mq_msgsize + strlen(clients_list[client_no].name + 1 + 100)];

		if(strcmp(buff, "quit") == 0)//client left
		{
			mq_close(clients_list[client_no].queue);
			clients_list[client_no].queue = -1;
			sprintf(message, "server:  %s has left\n", clients_list[client_no].name);//msg that client left
			need_to_notify = 0;//no need to notify
		}
		else//normal, not ending msg
		{
			sprintf(message, "%s@%s:\t%s\n", ctime(&msg_time), clients_list[client_no].name, buff);
		}

		printf("%s\n", message);
		int i;
		for(i = 0; i < actual_clients_number; i++)
		{
			if(i != client_no)
			{
				if(mq_send(clients_list[i].queue, message, strlen(message) + 1, (unsigned int)0) < 0 && errno != EBADF)
				{
					ERROR_HANDLER("receive_msg_handler");
				}
			}
		}
	}

	if(need_to_notify)
	{
		if(mq_notify(clients_list[client_no].queue, &clients_list[client_no].sigev) < 0)
		{
			ERROR_HANDLER("msg_client_handler");
		}
	}
}

void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	mq_unlink(SERVER_QUEUE);
}
