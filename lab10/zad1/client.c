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

static void set_client_name_path(char * name, char * path);
static void list_clients(char clients[MAX_CLIENTS][CLIENT_NAME_LENGTH]);
static void *thread_run(void *arg);
static void sigint_handler(int signo);
static void atexit_fun();

static int unix_type = 1;
static int server_port;
int client_port;
static char server_path[CLIENT_NAME_LENGTH + 2];
static char client_path[256];
static char *ip;
static int socket_descriptor;
static pthread_t thread;
static int is_thread_alive = 1;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static char client_name[CLIENT_NAME_LENGTH];

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

	atexit(atexit_fun);

	if(signal(SIGINT, sigint_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	printf("Creating client's name");
	set_client_name_path(client_name, client_path);
	OK();
	printf("Client's name:\t%s\n", client_name);

	printf("Creating socket");
	if(unix_type) 
	{
		if((socket_descriptor = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
		{
			ERROR_HANDLER("socket");
		}
	}
	else 
	{
		if((socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{
			ERROR_HANDLER("socket");
		}
	}
	OK();

	struct sockaddr_un	server_unix_address;
	struct sockaddr_un	client_unix_address;
	struct sockaddr_in	server_inet_address;
	struct sockaddr_in	client_inet_address;
	struct sockaddr* server_address;
	struct sockaddr* client_address;

	socklen_t unix_address_size = sizeof(struct sockaddr_un);
	socklen_t inet_address_size = sizeof(struct sockaddr_in);
	socklen_t address_size;

	memset(&server_unix_address, 0, sizeof(server_unix_address));
	memset(&client_unix_address, 0, sizeof(client_unix_address));
	memset(&server_inet_address, 0, sizeof(server_inet_address));
	memset(&client_inet_address, 0, sizeof(client_inet_address));

	if(unix_type) 
	{
		printf("Setting up server adress");
		server_unix_address.sun_family = AF_UNIX;
		strcpy(server_unix_address.sun_path, server_path);
		server_address = (struct sockaddr*) &server_unix_address;
		OK();

		printf("Setting up client adress");
		client_unix_address.sun_family = AF_UNIX;
		strcpy(client_unix_address.sun_path, client_path);
		client_address = (struct sockaddr*) &client_unix_address;
		OK();

		address_size = unix_address_size;
	}
	else 
	{
		printf("Setting up server adress");
		server_inet_address.sin_family = AF_INET;
		inet_pton(AF_INET, ip, &(server_inet_address.sin_addr.s_addr));
		server_inet_address.sin_port = htons(server_port);
		server_address = (struct sockaddr*) &server_inet_address;
		OK();

		printf("Setting up client adress");
		client_inet_address.sin_family = AF_INET;
		client_inet_address.sin_addr.s_addr = htonl(INADDR_ANY);
		client_inet_address.sin_port = htons(1024 + rand() % (65535 - 1024));
		client_address = (struct sockaddr*) &client_inet_address;
		OK();

		address_size = inet_address_size;
	}

	printf("Binding client socket");
	if(bind(socket_descriptor, client_address, address_size) == -1)
	{
		ERROR_HANDLER("bind()");
	}
	OK();

	//thread for responses
	if(pthread_create(&thread, NULL, thread_run, NULL) == -1)
	{
		ERROR_HANDLER("pthread_create()");
	}

	//clients registration
	request_t request;
	strcpy(request.name, client_name);
	request.action = ADD_CLIENT;

	printf("Sending registration request");
	pthread_mutex_lock(&mutex);

	if(sendto(socket_descriptor, (void*) &request, sizeof(request), 0, server_address, address_size) == -1)
	{
		ERROR_HANDLER("sendto()");
	}
	OK();

	printf("Receiving response from server");
	fflush(stdout);
	clients_list_t response;
	if(recv(socket_descriptor, (void*) &response, sizeof(response), 0) == -1)
	{
		ERROR_HANDLER("recv()");
	}
	OK();

	pthread_mutex_unlock(&mutex);
	printf("\nClient registered\nClients list:\n");
	list_clients(response.list);

	while(true) 
	{
		int action;
		printf("\nChoose action:\n\t"
				"1 - check clients list\n\t"
				"2 - check client's state\n\t"
				"3 - logout\n");
		scanf("%d", &action);

		if(action == 1) //print users list
		{
			request.action = LIST_CLIENTS;
			pthread_mutex_lock(&mutex);

			if(sendto(socket_descriptor, (void*) &request, sizeof(request), 0, server_address, address_size) == -1)
			{
				ERROR_HANDLER("sendto(list clients)");
			}

			if(recv(socket_descriptor, (void*) &response, sizeof(response), 0) == -1)
			{
				ERROR_HANDLER("recv()");
			}

			pthread_mutex_unlock(&mutex);
			printf("\nClients list:\n");
			list_clients(response.list);
		}
		else if(action == 2) //print user's state
		{
			printf("Which client would you like to check (type number): ");
			scanf("%d", &(request.client_no));
			request.action = CLIENT_INFO;
			client_info_t info;
			pthread_mutex_lock(&mutex);

			if(sendto(socket_descriptor, (void*) &request, sizeof(request), 0, server_address, address_size) == -1)
			{
				ERROR_HANDLER("sendto(client info)");
			}

			if(recv(socket_descriptor, (void*) &info, sizeof(info), 0) == -1)
			{
				ERROR_HANDLER("recv(client info)");
			}

			pthread_mutex_unlock(&mutex);

			if(info.is_info_successfull) 
			{
				printf("\nStats for client: %s\n", info.name);
				printf("\tProcesses:\t%d\n", info.proc_count);
				printf("\tLoad avg:\t%.2f\n", info.load_avg);
				printf("\tUsed mem:\t%d MB\n", info.mem_used);
				printf("\tFree mem:\t%d MB\n", info.mem_free);
			}
			else 
			{
				printf("\nClient is not responding. Can't show stats\n");
			}
		}
		else if(action == 3) //unregister
		{
			request.action = UNREGISTER;
			pthread_mutex_lock(&mutex);

			if(sendto(socket_descriptor, (void*) &request, sizeof(request), 0, server_address, address_size) == -1)
			{
				ERROR_HANDLER("sendto(unregister)");
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

static void set_client_name_path(char* name, char* path) 
{
	char host_name[CLIENT_NAME_LENGTH];
	char domain_name[CLIENT_NAME_LENGTH];
	char current_dir[256];

	FILE * fp = popen("hostname", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen - hostname");
	}

	fscanf(fp, "%s", host_name);
	pclose(fp);

	fp = popen("domainname", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen - domainname");
	}

	fscanf(fp, "%s", domain_name);
	pclose(fp);

	fp = popen("pwd", "r");
	if(fp == NULL)
	{
		ERROR_HANDLER("popen - pwd");
	}

	fscanf(fp, "%s", current_dir);
	pclose(fp);

	sprintf(name, "%s.%s", host_name, domain_name);
	sprintf(path, "%s/%s", current_dir, name);
}

static void list_clients(char clients[MAX_CLIENTS][CLIENT_NAME_LENGTH])
{
	int i;
	for(i = 0; i < MAX_CLIENTS && strlen(clients[i]) > 0; ++i)
	{
		printf("\t%2d. %s\n", i, clients[i]);
	}
}

static void *thread_run(void *arg)
{
	request_t request;
	socklen_t addrlen;
	struct sockaddr* src_addr;
	client_info_t info;
	FILE * fp;
	char trash[200];
	int int_trash;
	struct sockaddr_un unix_src_addr;
	struct sockaddr_in inet_src_addr;

	if(unix_type)
	{
		src_addr = (struct sockaddr*) &unix_src_addr;
		addrlen = sizeof(unix_src_addr);
	}
	else
	{
		src_addr = (struct sockaddr*) &inet_src_addr;
		addrlen = sizeof(inet_src_addr);
	}

	while(is_thread_alive)
	{
		pthread_mutex_lock(&mutex);

		if(recvfrom(socket_descriptor, (void *)&request, sizeof(request), MSG_DONTWAIT, src_addr, &addrlen) == -1) 
		{
			if(errno != EAGAIN && errno != EWOULDBLOCK)
			{
				ERROR_HANDLER("recvfrom");
			}
		}
		else if(request.action == CLIENT_INFO)
		{
			strcpy(info.name, client_name);

			if((fp = popen("ps -e | wc -l", "r")) == NULL)
			{
				ERROR_HANDLER("popen - ps -le");
			}
			fscanf(fp, "%d", &(info.proc_count));
			info.proc_count--;
			pclose(fp);

			if((fp = fopen("/proc/loadavg", "r")) == NULL)
			{
				ERROR_HANDLER("popen - /proc/loadavg");
			}
			fscanf(fp, "%f", &(info.load_avg));
			fclose(fp);

			if((fp = popen("free -m", "r")) == NULL)
			{
				ERROR_HANDLER("popen - free");
			}
			fscanf(fp,"%s %s %s %s %s %s %s", trash, trash, trash, trash, trash, trash, trash);
			fscanf(fp, "%d %d %d %s\n", &int_trash, &(info.mem_used), &(info.mem_free), trash);
			pclose(fp);

			if(sendto(socket_descriptor, (void*) &info, sizeof(info), 0, src_addr, addrlen) == -1)
			{
				ERROR_HANDLER("sendto");
			}
		}
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

static void sigint_handler(int signo)
{
	if(signo == SIGINT)
	{
		fprintf(stderr, "\nExited normally.\n");
		exit(EXIT_SUCCESS);
	}
}

static void atexit_fun()
{
	is_thread_alive = 0;
	if(thread != 0)
	{
		pthread_join(thread, NULL);
	}

	close(socket_descriptor);
	remove(client_path);
	fprintf(stderr, "Resources closed.\n");
}
