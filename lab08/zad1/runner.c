#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s producers_number consumers_number\n", name)
#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

#define BEGIN_DELAY_SECONDS 5

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	int producers = atoi(argv[1]);
	int consumers = atoi(argv[2]);

	if(producers <= 0 || consumers <= 0)
	{
		fprintf(stderr, "Both producers_number and consumers_number must be positive integer.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	pid_t *running_processes = malloc((producers + consumers) * sizeof(pid_t));
	if(running_processes == NULL)
	{
		ERROR_HANDLER("malloc");
	}

	pid_t pid;
	char id_string[10];

	printf("Program will start in %d seconds. To exit press 'Enter'.\n", BEGIN_DELAY_SECONDS);
	sleep(BEGIN_DELAY_SECONDS);

	for(int i = 0; i < producers; ++i)
	{
		pid = fork();
		sprintf(id_string, "%d", i);
		if(pid == -1)
		{
			ERROR_HANDLER("producer fork failure");
		}
		else if(pid == 0)
		{
			execl("./producer", "producer", id_string, (char *)NULL);
		}
		else
		{
			running_processes[i] = pid;
		}
	}

	for(int i = 0; i < consumers; ++i)
	{
		pid = fork();
		sprintf(id_string, "%d", i);
		if(pid == -1)
		{
			ERROR_HANDLER("consumer fork failure");
		}
		else if(pid == 0)
		{
			execl("./consumer", "consumer", id_string, (char *)NULL);
		}
		else
		{
			running_processes[producers + i] = pid;
		}
	}

	//waiting for 'Enter'
	getchar();

	for(int i = 0; i < producers + consumers; ++i)
	{
		if(kill(running_processes[i], SIGINT) == -1)
		{
			ERROR_HANDLER("kill");
		}
	}

	return 0;
}
