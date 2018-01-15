#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s port path\n", name);

static void send_clients_list(int sockfd, const struct sockaddr *dest_addr, socklen_t addrlen);
static void copy_clients(char dest[MAX_CLIENTS][CLIENT_NAME_LENGTH], client_t src[MAX_CLIENTS]);
static int add_client(char name[CLIENT_NAME_LENGTH], int is_local, struct sockaddr* addr, socklen_t addrlen) ;
static int delete_client(char name[CLIENT_NAME_LENGTH]);
static void sigint_handler(int sig);
static void atexit_fun();

static client_t clients[MAX_CLIENTS];
static int local_flag = 1;
static int port;
static char path[100];

static int unix_socket;
static int inet_socket;

static struct sockaddr_un	server_unix_address;
static struct sockaddr_un	client_unix_address;
static struct sockaddr_in	server_inet_address;
static struct sockaddr_in	client_inet_address;

static socklen_t unix_address_size = sizeof(struct sockaddr_un);
static socklen_t inet_address_size = sizeof(struct sockaddr_in);

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		USAGE;
		exit(EXIT_FAILURE);
	}

	atexit(atexit_fun);

	port = atoi(argv[1]);
	strcpy(path, argv[2]);

	if(signal(SIGINT, sigint_handler) == SIG_ERR)
	{
		ERROR_HANDLER("signal");
	}

	printf("\nCreating server socket for local communication");
	if((unix_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
	{
		ERROR_HANDLER("socket(server unix)");
	}
	OK();

	printf("Creating server socket for internet communication");
	if((inet_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
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
		ERROR_HANDLER("bind(unix)");
	}

	if(bind(inet_socket, (struct sockaddr*) &server_inet_address, inet_address_size) == -1)
	{
		ERROR_HANDLER("bind(inet)");
	}
	OK();

	printf("Creating data structures");
	int i;
	for(i = 0; i < MAX_CLIENTS; ++i)
	{
		clients[i].name[0] = '\0';
	}

	request_t request;
	OK();

	while(true)
	{
		printf("\nWaiting for new request");
		fflush(stdout);
		unix_address_size = sizeof(struct sockaddr_un);
		inet_address_size = sizeof(struct sockaddr_in);

		while(true)
		{
			if(recvfrom(unix_socket, (void*) &request, sizeof(request), MSG_DONTWAIT,
						(struct sockaddr*) &client_unix_address, &unix_address_size) > 0)
			{
				local_flag = 1;
				break;
			}

			if(recvfrom(inet_socket, (void*) &request, sizeof(request), MSG_DONTWAIT,
						(struct sockaddr*) &client_inet_address, &inet_address_size) > 0)
			{
				local_flag = 0;
				break;
			}

			usleep(1000);
		}

		OK();

		if(request.action == ADD_CLIENT) 
		{
			printf("Registering client ");
			if(local_flag)
			{
				add_client(request.name, local_flag, (struct sockaddr*) &client_unix_address, unix_address_size);
			}
			else
			{
				add_client(request.name, local_flag, (struct sockaddr*) &client_inet_address, inet_address_size);
			}

			OK();

			printf("Sending back clients list");

			if(local_flag)
			{
				send_clients_list(unix_socket, (struct sockaddr*) &client_unix_address, unix_address_size);
			}
			else
			{
				send_clients_list(inet_socket, (struct sockaddr*) &client_inet_address, inet_address_size);
			}

			OK();
		}
		else if(request.action == UNREGISTER)
		{
			printf("Unregistering client: %s", request.name);
			delete_client(request.name);
		}
		else if(request.action == LIST_CLIENTS)
		{
			printf("Sending clients list to: %s", request.name);

			if(local_flag)
			{
				send_clients_list(unix_socket, (struct sockaddr*) &client_unix_address, unix_address_size);
			}
			else
			{
				send_clients_list(inet_socket, (struct sockaddr*) &client_inet_address, inet_address_size);
			}
		}
		else if (request.action == CLIENT_INFO)
		{
			printf("CLIENT_INFO request received\n");

			request_t info_request;
			info_request.action = CLIENT_INFO;
			int sockfd;
			struct sockaddr *dest_addr;
			socklen_t addrlen;
			int client_no = request.client_no;

			addrlen = clients[client_no].address_size;

			if(clients[client_no].is_local)
			{
				sockfd = unix_socket;
				dest_addr = (struct sockaddr *)&(clients[client_no].unix_address);

				printf("Sending CLIENT_INFO request forward to local client:\n"
						"path: %s\naddress size: %d\n", clients[client_no].unix_address.sun_path, addrlen);
			}
			else
			{
				sockfd = inet_socket;
				dest_addr = (struct sockaddr *)&(clients[client_no].inet_address);

				char ip[20];
				inet_ntop(AF_INET, &(clients[client_no].inet_address.sin_addr.s_addr), ip, 20);

				printf("Sending CLIENT_INFO request forward to remote client:\n"
						"ip: %s\nport: %d\naddress size: %d\n",
						ip, clients[client_no].inet_address.sin_port, addrlen);
			}

			if(sendto(sockfd, (void*) &info_request, sizeof(info_request), 0, dest_addr, addrlen) == -1)
			{
				ERROR_HANDLER("main::sendto");
			}

			client_info_t info;
			int trials_conuter = 0;
			bool info_received = false;

			while(trials_conuter < 2000 && !info_received)
			{
				usleep(1000);

				if(recv(sockfd, (void *)&info, sizeof(info), MSG_DONTWAIT) == -1)
				{
					if(errno != EAGAIN && errno != EWOULDBLOCK)
					{
						ERROR_HANDLER("main::recv");
					}
					else
					{
						++trials_conuter;
						continue;
					}
				}

				info_received = true;
			}

			if(!info_received)
			{
				printf("Client not responding\n");
				info.is_info_successfull = false;
			}

			if(local_flag)//sending response
			{
				if(sendto(unix_socket, (void *)&info, sizeof(info), 0, (struct sockaddr *)&client_unix_address, unix_address_size) == -1)
				{
					ERROR_HANDLER("main::sendto");
				}
			}
			else
			{
				if(sendto(inet_socket, (void *)&info, sizeof(info), 0, (struct sockaddr *)&client_inet_address, inet_address_size) == -1)
				{
					ERROR_HANDLER("main::sendto");
				}
			}
		}
		else
		{
			printf("Request received but not recognized: %d\n", request.action);
		}
	}

	exit(EXIT_SUCCESS);
}

static void send_clients_list(int sockfd, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	clients_list_t response;
	copy_clients(response.list, clients);
	if(sendto(sockfd, (void *)&response, sizeof(response), 0, dest_addr, addrlen) == -1)
	{
		ERROR_HANDLER("send_clients_list::sendto");
	}
}

static void copy_clients(char dest[MAX_CLIENTS][CLIENT_NAME_LENGTH], client_t src[MAX_CLIENTS])
{
	int i;
	for(i = 0; i < MAX_CLIENTS; ++i)
	{
		strcpy(dest[i], src[i].name);
	}
}

static int add_client(char name[CLIENT_NAME_LENGTH], int is_local, struct sockaddr* addr, socklen_t addrlen)
{
	int i = 0;
	while(strlen(clients[i].name) > 0 && i < MAX_CLIENTS)
	{
		++i;
	}

	if(i < MAX_CLIENTS)
	{
		strcpy(clients[i].name, name);
		clients[i].is_local = is_local;
		clients[i].address_size = addrlen;

		if(is_local)
		{
			memcpy(&(clients[i].unix_address), (struct sockaddr_un*)addr, addrlen);
		}
		else
		{
			memcpy(&(clients[i].inet_address), (struct sockaddr_in*)addr, addrlen);
		}

		return true;
	}

	return false;
}

static int delete_client(char name[CLIENT_NAME_LENGTH])
{
	int i = 0;
	while(strcmp(clients[i].name, name) && i < MAX_CLIENTS)
	{
		++i;
	}

	if(i >= MAX_CLIENTS)
	{
		return false;
	}

	while(i + 1 < MAX_CLIENTS)
	{
		strcpy(clients[i].name, clients[i + 1].name);
		clients[i].is_local = clients[i + 1].is_local;
		clients[i].address_size = clients[i + 1].address_size;

		if(clients[i + 1].is_local)
		{
			memcpy(&(clients[i].unix_address), &(clients[i + 1].unix_address), clients[i + 1].address_size);
		}
		else
		{
			memcpy(&(clients[i].inet_address), &(clients[i + 1].inet_address), clients[i + 1].address_size);
		}
		
		++i;
	}

	clients[i].name[0] = (char) 0;
	return true;
}

static void sigint_handler(int sig)
{
	fprintf(stderr, "\nExited normally.\n");
	exit(EXIT_SUCCESS);
}

static void atexit_fun()
{
	close(unix_socket);
	close(inet_socket);
	remove(path);
	fprintf(stderr, "Resources closed.\n");
}
