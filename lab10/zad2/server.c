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
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s port path\n", name);

static void *thread_run(void *arg);
static int get_client_number(client_t* client);
static int get_actual_clients_quantity();
static void send_clients_list(int sockfd);
static void sigint_handler(int sig);
static void atexit_function();

static int port;
static char path[100];
static client_t *head_client;
static client_info_t shared_info;
static pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t info_producer_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t info_consumer_mutex = PTHREAD_MUTEX_INITIALIZER;

static int unix_socket;
static int inet_socket;

static struct sockaddr_un	server_unix_address;
static struct sockaddr_un	client_unix_address;
static struct sockaddr_in	server_inet_address;
static struct sockaddr_in	client_inet_address;

static socklen_t unix_address_size = sizeof(struct sockaddr_un);
static socklen_t inet_address_size = sizeof(struct sockaddr_in);

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		USAGE;
		exit(EXIT_FAILURE);
	}

	atexit(atexit_function);

	port = atoi(argv[1]);
	strcpy(path, argv[2]);

	if(signal(SIGINT, sigint_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	printf("\nCreating server socket for local communication");
	if((unix_socket = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
	{
		ERROR_HANDLER("socket(server unix)");
	}
	OK();

	printf("Creating server socket for internet communication");
	if((inet_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
	{
		ERROR_HANDLER("socket(server inet)");
	}
	OK();

	memset(&server_unix_address, 0, sizeof(server_unix_address));
	memset(&server_inet_address, 0, sizeof(server_inet_address));
	memset(&client_unix_address, 0, sizeof(client_unix_address));
	memset(&client_inet_address, 0, sizeof(client_inet_address));

	server_unix_address.sun_family = AF_UNIX;
	strcpy(server_unix_address.sun_path, path);

	server_inet_address.sin_family = AF_INET;
	server_inet_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_inet_address.sin_port = htons(port);

	printf("Binding sockets");
	
	if(bind(unix_socket, (struct sockaddr*) &server_unix_address, unix_address_size) == -1)
	{
		ERROR_HANDLER("bind");
	}
	
	if(bind(inet_socket, (struct sockaddr*) &server_inet_address, inet_address_size) == -1)
	{
		ERROR_HANDLER("bind");
	}
	OK();

	if(listen(unix_socket, 10) == -1)
	{
		ERROR_HANDLER("listen");
	}
	if(listen(inet_socket, 10) == -1)
	{
		ERROR_HANDLER("listen");
	}

	pthread_mutex_lock(&info_consumer_mutex);

	printf("Waiting for clients\n");
	client_t tmp_client;
	
	while(true) 
	{
		usleep(1000);
		pthread_mutex_lock(&client_list_mutex);
	
		tmp_client.socket = accept(unix_socket, NULL, NULL);
		if(tmp_client.socket == -1) 
		{
			if(tmp_client.socket == EAGAIN || tmp_client.socket == EWOULDBLOCK)
			{
				ERROR_HANDLER("accept");
			}

			pthread_mutex_unlock(&client_list_mutex);
		}
		else 
		{
			if(pthread_create(&tmp_client.thread, NULL, thread_run, &tmp_client) == -1)
			{
				ERROR_HANDLER("pthread_create");
			}
		}

		pthread_mutex_lock(&client_list_mutex);
		tmp_client.socket = accept(inet_socket, NULL, NULL);
		if(tmp_client.socket == -1) 
		{
			if(tmp_client.socket == EAGAIN || tmp_client.socket == EWOULDBLOCK)
			{
				ERROR_HANDLER("accept");
			}
			pthread_mutex_unlock(&client_list_mutex);
		}
		else 
		{
			if(pthread_create(&tmp_client.thread, NULL, thread_run, &tmp_client) == -1)
			{
				ERROR_HANDLER("pthread_create");
			}
		}
	}

	exit(EXIT_SUCCESS);
}

static void *thread_run(void *tmp_client) 
{
	printf("\nNew thread created\n");
	client_t client;
	client.socket = ((client_t*) tmp_client)->socket;
	client.thread = ((client_t*) tmp_client)->thread;

	if(get_actual_clients_quantity() >= MAX_CLIENTS) 
	{
		printf("Too many clients ==> canceling thread\n");
		pthread_mutex_unlock(&client_list_mutex);
		if(pthread_cancel(client.thread) == -1)
		{
			ERROR_HANDLER("pthread_cancel");
		}
	}

	client.next = head_client;
	head_client = &client;

	request_t request;

	while(true) 
	{

		if(recv(client.socket, (void*) &request, sizeof(request), 0) == -1)
		{
			ERROR_HANDLER("recv");
		}

		printf("\nReceived request %d from #%d\n", request.action, get_client_number(&client));

		if(request.action == MY_NAME_IS) 
		{
			printf("Registering client ");

			strcpy(client.name, request.name);
			send_clients_list(client.socket);

			OK();
			pthread_mutex_unlock(&client_list_mutex);
		}
		else if(request.action == UNREGISTER) 
		{
			printf("Unregistering client: %s\n", client.name);
			pthread_mutex_lock(&client_list_mutex);

			if(head_client == &client)
			{
					head_client = client.next;
			}
			else 
			{
				client_t* prev = head_client;
				while(prev->next != &client && prev->next != NULL)
				{
					prev = prev->next;
				}

				if(prev->next != NULL)
				{
					prev->next = client.next;
				}
			}

			pthread_mutex_unlock(&client_list_mutex);

			if(pthread_cancel(client.thread) == -1)
			{
				ERROR_HANDLER("thread_run() --> pthread_cancel()");
			}
		}
		else if(request.action == LIST_CLIENTS) 
		{
			printf("Sending clients list to %s\n", request.name);

			pthread_mutex_lock(&client_list_mutex);
			send_clients_list(client.socket);
			pthread_mutex_unlock(&client_list_mutex);
		}
		else if (request.action == CLIENT_INFO) 
		{
			printf("CLIENT_INFO request received\n");

			request_t info_request;
			info_request.action = CLIENT_INFO;
			pthread_mutex_lock(&client_list_mutex);
			
			client_t* tmp = head_client;
			int i;
			for(i = 0; i < request.client_no && tmp != NULL; i++)
			{
				tmp = tmp->next;
			}

			int sockfd = tmp->socket;
			
			pthread_mutex_unlock(&client_list_mutex);

			client_info_t info;
			int info_received = false;
			if(tmp == NULL) 
			{
				printf("Requested client not found\n");
			}
			else 
			{
				printf("Sending CLIENT_INFO forward to: %s\n", tmp->name);
				if(send(sockfd, (void*) &info_request, sizeof(info_request), 0) == -1)
				{
					ERROR_HANDLER("send");
				}
				int trials_conuter = 0;
				while(trials_conuter < 2000 && !info_received) 
				{
					if((errno = pthread_mutex_trylock(&info_consumer_mutex)) != 0) 
					{
						if(errno == EBUSY) 
						{
							++trials_conuter;
							usleep(1000);
							continue;
						}
						else
						{
							ERROR_HANDLER("pthread_mutex_trylock");
						}
					}

					//if msg received successfully
					info_received = true;
				}

				printf("trial countr = %d\n", trials_conuter);

				printf("Response %s received after %d ms\n", (info_received) ? "" : " not", trials_conuter);
				if(!info_received) 
				{
					printf("Client is not responding\n");
					info.is_info_successfull = false;
				}
				else 
				{
					info.is_info_successfull = true;
					info.load_avg = shared_info.load_avg;
					info.mem_free = shared_info.mem_free;
					info.mem_used = shared_info.mem_used;
					info.proc_count = shared_info.proc_count;
				}
			}
			//sending response
			if(send(client.socket, (void*) &info, sizeof(info), 0) == -1)
			{
				ERROR_HANDLER("send(info_request)");
			}

			pthread_mutex_unlock(&info_producer_mutex);
		}
		else if(request.action == INFO_RESPONSE) 
		{
			pthread_mutex_lock(&info_producer_mutex);
			strcpy(shared_info.name, request.info.name);
			shared_info.load_avg = request.info.load_avg;
			shared_info.mem_free = request.info.mem_free;
			shared_info.mem_used = request.info.mem_used;
			shared_info.proc_count = request.info.proc_count;
			pthread_mutex_unlock(&info_consumer_mutex);
		}
		else
		{
			printf("Request received but not recognized: %d\n", request.action);
		}
	}

	exit(EXIT_SUCCESS);
}

static int get_client_number(client_t *client) 
{
	client_t* tmp = head_client;
	int i = 0;
	while(tmp != client && tmp != NULL) 
	{
		tmp = tmp->next;
		i++;
	}

	return (tmp == NULL) ? -1 : i;
}

static int get_actual_clients_quantity() 
{
	int result = 0;

	client_t* tmp = head_client;
	while(tmp != NULL) 
	{
		tmp = tmp->next;
		++result;
	}

	return result;
}

static void send_clients_list(int sockfd) 
{
	clients_list_t response;
	memset(&response, 0, sizeof(response));

	int i = 0;
	client_t *tmp = head_client;
	while(tmp != NULL && i < MAX_CLIENTS) 
	{
		strcpy(response.list[i], tmp->name);
		tmp = tmp->next;
		++i;
	}

	if(send(sockfd, (void *)&response, sizeof(response), 0) == -1)
	{
		ERROR_HANDLER("send");
	}
}

static void sigint_handler(int sig) 
{
	fprintf(stderr, "\nExited normally.\n");
	exit(EXIT_SUCCESS);
}

static void atexit_function()
{
	close(unix_socket);
	close(inet_socket);
	remove(path);
	fprintf(stderr, "Resources closed.\n");
}
