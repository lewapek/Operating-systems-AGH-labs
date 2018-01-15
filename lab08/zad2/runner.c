#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s writers_number readers_number\n", name)
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

	int writers = atoi(argv[1]);
	int readers = atoi(argv[2]);

	if(writers <= 0 || readers <= 0)
	{
		fprintf(stderr, "Both writers_number and readers_number must be positive integer.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	pid_t *running_processes = malloc((writers + readers) * sizeof(pid_t));
	if(running_processes == NULL)
	{
		ERROR_HANDLER("malloc");
	}

	pid_t pid;
	char id_string[10];

	printf("Program will start in %d seconds. To exit press 'Enter'.\n", BEGIN_DELAY_SECONDS);
	sleep(BEGIN_DELAY_SECONDS);

	for(int i = 0; i < writers; ++i)
	{
		pid = fork();
		sprintf(id_string, "%d", i);
		if(pid == -1)
		{
			ERROR_HANDLER("writer fork failure");
		}
		else if(pid == 0)
		{
			execl("./writer", "writer", id_string, (char *)NULL);
		}
		else
		{
			running_processes[i] = pid;
		}
	}

	char min[10], max[10];
	for(int i = 0; i < readers; ++i)
	{
		pid = fork();
		sprintf(id_string, "%d", i);
		if(pid == -1)
		{
			ERROR_HANDLER("reader fork failure");
		}
		else if(pid == 0)
		{
			sprintf(min, "%d", -1 - rand() % 10);
			sprintf(max, "%d", 1 + rand() % 10);
			execl("./reader", "reader", id_string, min, max, (char *)NULL);
		}
		else
		{
			running_processes[writers + i] = pid;
		}
	}

	//waiting for 'Enter'
	getchar();

	for(int i = 0; i < writers + readers; ++i)
	{
		if(kill(running_processes[i], SIGINT) == -1)
		{
			ERROR_HANDLER("kill");
		}
	}

	return 0;
}
