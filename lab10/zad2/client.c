#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s (0 | 1) (path | ip_address port)\n\t0 - unix socket\n\t1 - internet socket\n", name);

static int clients_max_index();
static void create_client_name_and_set_client_path(char *name, char *path);
static void list_clients(char clients[MAX_CLIENTS][CLIENT_NAME_LENGTH]);
static void *thread_run(void *arg);
static void sigint_handler(int sig);
static void atexit_function();

static int unix_type = 1;
static int server_port;
static char server_path[CLIENT_NAME_LENGTH + 2];
static char client_path[256];
static char *ip;
static int socket_fd;
static pthread_t thread;
static int thread_is_alive = 1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static char client_name[CLIENT_NAME_LENGTH];
static clients_list_t latest_clients_list;

int main(int argc, char *argv[])
{
	if(argc < 3)
	{
		USAGE;
		exit(EXIT_FAILURE);
	}

	if(strcmp(argv[1], "0") == 0) 
	{
		if(argc != 3) 
		{
			USAGE;
			exit(EXIT_FAILURE);
		}
		unix_type = 1;
		strcpy(server_path, argv[2]);
	}
	else if(strcmp(argv[1], "1") == 0) 
	{
		if(argc != 4) 
		{
			USAGE;
			exit(EXIT_FAILURE);
		}

		unix_type = 0;
		ip= argv[2];
		server_port = atoi(argv[3]);
	}
	else 
	{
		USAGE;
		exit(EXIT_FAILURE);
	}

	atexit(atexit_function);

	if(signal(SIGINT, sigint_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	printf("Creating client's name");
	create_client_name_and_set_client_path(client_name, client_path);
	OK();
	printf("Client's name:\t%s\n", client_name);

	printf("Creating socket");
	if(unix_type) {
		if((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
			ERROR_HANDLER("socket()");
	}
	else {
		if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			ERROR_HANDLER("socket()");
	}
	OK();

	struct sockaddr_un	server_unix_address;
	struct sockaddr_in	server_inet_address;
	struct sockaddr* server_address;

	socklen_t unix_address_size = sizeof(struct sockaddr_un);
	socklen_t inet_address_size = sizeof(struct sockaddr_in);
	socklen_t address_size;

	memset(&server_unix_address, 0, sizeof(server_unix_address));
	memset(&server_inet_address, 0, sizeof(server_inet_address));

	if(unix_type) 
	{
		printf("Setting up server adress - AF_UNIX...");
		server_unix_address.sun_family = AF_UNIX;
		strcpy(server_unix_address.sun_path, server_path);
		server_address = (struct sockaddr *)&server_unix_address;
		OK();

		address_size = unix_address_size;
	}
	else 
	{
		printf("Setting up server adress - AF_INET...");
		server_inet_address.sin_family = AF_INET;
		inet_pton(AF_INET, ip, &(server_inet_address.sin_addr.s_addr));
		server_inet_address.sin_port = htons(server_port);
		server_address = (struct sockaddr*) &server_inet_address;
		OK();

		address_size = inet_address_size;
	}

	printf("Connecting to server...");
	if(connect(socket_fd, server_address, address_size) == -1)
	{
		ERROR_HANDLER("connect");
	}
	OK();

	//thread for responses
	if(pthread_create(&thread, NULL, thread_run, NULL) == -1)
	{
		ERROR_HANDLER("pthread_create");
	}

	//clients registration
	request_t request;
	strcpy(request.name, client_name);
	request.action = MY_NAME_IS;

	printf("Sending registration request...");
	pthread_mutex_lock(&mutex);

	if(send(socket_fd, (void*) &request, sizeof(request), 0) == -1)
	{
		ERROR_HANDLER("send");
	}
	OK();

	printf("Getting response from server...");
	fflush(stdout);
	clients_list_t response;
	if(recv(socket_fd, (void*) &response, sizeof(response), 0) == -1)
	{
		ERROR_HANDLER("recv()");
	}
	OK();

	pthread_mutex_unlock(&mutex);
	printf("\nClient registered successfully\n");
	
	list_clients(response.list);
	memcpy(&latest_clients_list, response.list, sizeof(clients_list_t));

	while(true) 
	{
		int action;
		printf("\nChoose one:\n\t"
			   "1 - clients list\n\t"
			   "2 - client's state\n\t"
			   "3 - logout\n");
		scanf("%d", &action);

		if(action == 1)
		{
			request.action = LIST_CLIENTS;
			pthread_mutex_lock(&mutex);

			if(send(socket_fd, (void *)&request, sizeof(request), 0) == -1)
			{
				ERROR_HANDLER("send");
			}

			if(recv(socket_fd, (void *)&response, sizeof(response), 0) == -1)
			{
				ERROR_HANDLER("recv");
			}

			pthread_mutex_unlock(&mutex);
			
			list_clients(response.list);
			memcpy(&latest_clients_list, response.list, sizeof(clients_list_t));
		}
		else if(action == 2)//printing user's state
		{
			list_clients(latest_clients_list.list);
			printf("\nWrite client number: ");
			scanf("%d", &(request.client_no));

			if(request.client_no > clients_max_index() || request.client_no < 0) 
			{
				printf("Invalid number!\n");
				continue;
			}

			request.action = CLIENT_INFO;
			client_info_t info;
			pthread_mutex_lock(&mutex);

			if(send(socket_fd, (void *)&request, sizeof(request), 0) == -1)
			{
				ERROR_HANDLER("send");
			}

			while(!info.is_info_successfull)
			{
			printf("inside while\n");	
			if(recv(socket_fd, (void *)&info, sizeof(info), 0) == -1)
			{
				ERROR_HANDLER("recv");
			}
			}

			pthread_mutex_unlock(&mutex);

			if(info.is_info_successfull) 
			{
				printf("\nStats for client %s\n", info.name);
				printf("\tProcesses:\t%d\n", info.proc_count);
				printf("\tLoad avg:\t%.2f\n", info.load_avg);
				printf("\tUsed mem:\t%d MB\n", info.mem_used);
				printf("\tFree mem:\t%d MB\n", info.mem_free);
			}
			else 
			{
				printf("\nClient is not responding. Unable to show stats\n");
			}
		}
		else if(action == 3)//unregister 
		{
			request.action = UNREGISTER;
			pthread_mutex_lock(&mutex);

			if(send(socket_fd, (void*) &request, sizeof(request), 0) == -1)
			{
				ERROR_HANDLER("send");
			}

			pthread_mutex_unlock(&mutex);

			break;
		}
		else 
		{
			printf("Invalid action, try again\n");
		}
	}

	exit(EXIT_SUCCESS);
}

static int clients_max_index() 
{
	int i = 0;
	while(i < MAX_CLIENTS && strlen(latest_clients_list.list[i]) > 0)
	{
		++i;
	}

	return i - 1;
}

static void create_client_name_and_set_client_path(char *name, char *path) 
{
	char host_name[CLIENT_NAME_LENGTH];
	char domain_name[CLIENT_NAME_LENGTH];
	char current_dir[256];

	FILE * fp = popen("hostname", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen");
	}

	fscanf(fp, "%s", host_name);
	pclose(fp);

	fp = popen("domainname", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen");
	}

	fscanf(fp, "%s", domain_name);
	pclose(fp);

	fp = popen("pwd", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen");
	}

	fscanf(fp, "%s", current_dir);
	pclose(fp);

	sprintf(name, "%s.%s", host_name, domain_name);
	sprintf(path, "%s/%s", current_dir, name);
}

static void list_clients(char clients[MAX_CLIENTS][CLIENT_NAME_LENGTH]) 
{
	printf("Clients list:\n");
	int i;
	for(i = 0; i < MAX_CLIENTS && strlen(clients[i]) > 0; ++i)
	{
		printf("\t%2d. %s\n", i, clients[i]);
	}
}

static void *thread_run(void *arg) 
{
	request_t request;
	request_t response;
	response.action = INFO_RESPONSE;
	FILE * fp;
	char trash[200];
	int int_trash;

	while(thread_is_alive) 
	{
		pthread_mutex_lock(&mutex);

		if(recv(socket_fd, (void*) &request, sizeof(request), MSG_DONTWAIT) == -1) 
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
				ERROR_HANDLER("recv");
			}
		}
		else if(request.action == CLIENT_INFO) 
		{
			strcpy(response.info.name, client_name);

			if((fp = fopen(request.text, "r")) == NULL)
			{
				ERROR_HANDLER("popen");
			}
			fscanf(fp, "%s", (response.text));
			fclose(fp);

			if(send(socket_fd, (void*) &response, sizeof(response), 0) == -1)
			{
				ERROR_HANDLER("send");
			}
		}

		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

static void sigint_handler(int sig) 
{
	exit(EXIT_SUCCESS);
}

static void atexit_function()
{
	thread_is_alive = 0;
	if(thread != 0)
	{
		pthread_join(thread, NULL);
	}
	
	close(socket_fd);
	remove(client_path);
}
