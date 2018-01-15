#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define CLIENT_NAME_LENGTH 50
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }
#define OK() printf("  [ DONE ]\n");

typedef enum action_t
{
	MY_NAME_IS,
	LIST_CLIENTS,
	CLIENT_INFO,
	UNREGISTER,
	INFO_RESPONSE
} action_t;

typedef struct client_t 
{
	char name[CLIENT_NAME_LENGTH];
	int socket;
	pthread_t thread;
	struct client_t *next;
} client_t;

typedef struct clients_list_t
{
	char list[MAX_CLIENTS][CLIENT_NAME_LENGTH];
} clients_list_t;

typedef struct client_info_t
{
	int is_info_successfull;
	char name[CLIENT_NAME_LENGTH];
	int proc_count;
	float load_avg;
	int mem_used;
	int mem_free;
} client_info_t;

typedef struct request_t
{
	char name[CLIENT_NAME_LENGTH];
	int client_no;
	action_t action;
	client_info_t info;
} request_t;

#endif/*_HEADER_H_*/
