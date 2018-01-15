#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s -k pid\n\t%s -s val pid\n", name, name);

static void ask_before_sending(int pid);
static void send_using_kill(int pid);
static void send_using_sigqueue(int pid, int val);

int main(int argc, char const *argv[])
{
	if(argc == 3)//kill
	{
		if(strcmp(argv[1], "-k") != 0)
		{
			fprintf(stderr, "Number of arguments is %d. Invalid option: %s.\n", argc, argv[1]);
			USAGE;
			exit(EXIT_FAILURE);
		}

		pid_t pid = (pid_t)atoi(argv[2]);
		if(pid <= 0)
		{
			fprintf(stderr, "Invalid argument 'pid'. Should be positive integer.\n");
			USAGE;
			exit(EXIT_FAILURE);
		}

		ask_before_sending(pid);
		send_using_kill(pid);
	}
	else if(argc == 4)//sigqueue
	{
		if(strcmp(argv[1], "-s") != 0)
		{
			fprintf(stderr, "Number of arguments is %d. Invalid option: %s.\n", argc, argv[1]);
			USAGE;
			exit(EXIT_FAILURE);
		}

		int val = atoi(argv[2]);
		if(val <= 0)
		{
			fprintf(stderr, "Invalid argument 'val'. Should be positive integer.\n");
			USAGE;
			exit(EXIT_FAILURE);
		}

		pid_t pid = (pid_t)atoi(argv[3]);
		if(pid <= 0)
		{
			fprintf(stderr, "Invalid argument 'pid'. Should be positive integer.\n");
			USAGE;
			exit(EXIT_FAILURE);
		}

		ask_before_sending(pid);
		send_using_sigqueue(pid, val);
	}
	else
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	return 0;
}

static void ask_before_sending(int pid)
{
	printf("Press any key to send SIGUSR1 to process with pid = %d...", pid);
	getchar();
}

static void send_using_kill(int pid)
{
	int is_send = kill(pid, SIGUSR1);
	if(is_send != 0)
	{
		perror("Problem with sending SIGUSR1 using kill");
		exit(EXIT_FAILURE);
	}
}

static void send_using_sigqueue(int pid, int val)
{
	union sigval value_union;
	value_union.sival_int = val;

	int is_send = sigqueue(pid, SIGUSR1, value_union);
	if(is_send != 0)
	{
		perror("Problem with sending SIGUSR1 using sigqueue");
		exit(EXIT_FAILURE);
	}
}
