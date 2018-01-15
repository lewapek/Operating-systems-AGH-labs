#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "header.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s id min max\n", name);

void task_execute();
void sigint_handler(int sig);
void atexit_function();

int *shared_array;
int *readers_count;
int sh_tab_fd;
int sh_count_fd;
sem_t *sem_writer;
sem_t *sem_reader;

int min, max;
bool counter_already_created = false;
long tasks_counter = 1;

int reader_id;

int main(int argc, char const *argv[])
{
	if(argc != 4)
	{
		fprintf(stderr, "Wrong number of arguments\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	reader_id = atoi(argv[1]);
	min = atoi(argv[2]);
	max = atoi(argv[3]);

	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	sh_tab_fd = shm_open(SHARED_MEM, O_RDONLY, 0);
	if(sh_tab_fd == -1)
	{
		ERROR_HANDLER("shm_open");
	}

	sh_count_fd = shm_open(READERS_COUNT, O_RDWR | O_CREAT | O_EXCL, 0600);
	if(sh_count_fd == -1)
	{
		if(errno == EEXIST)
		{
			counter_already_created = true;
		}
		else
		{
			ERROR_HANDLER("shm_open");
		}
	}
	if(counter_already_created)
	{
		sh_count_fd = shm_open(READERS_COUNT, O_RDWR, 0);
		if(sh_count_fd == -1)
		{
			ERROR_HANDLER("shm_open");
		}
	}
	else
	{
		if(ftruncate(sh_count_fd, sizeof(int)) == -1)
		{
			ERROR_HANDLER("ftruncate");
		}
	}

	shared_array = (int *) mmap(NULL, shared_memory_size, PROT_READ, MAP_SHARED, sh_tab_fd, 0);
	if(shared_array == MAP_FAILED)
	{
		ERROR_HANDLER("mmap");
	}

	readers_count = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, sh_count_fd, 0);
	if(readers_count == MAP_FAILED)
	{
		ERROR_HANDLER("mmap");
	}

	if(!counter_already_created)
	{
		*readers_count = 0;
	}

	sem_writer = sem_open(WRITERS_SEMAPHORE, 0);
	if(sem_writer == SEM_FAILED)
	{
		ERROR_HANDLER("sem_open");
	}

	sem_reader = sem_open(READERS_SEMAPHORE, O_CREAT, 0600, 1);
	if(sem_reader == SEM_FAILED)
	{
		ERROR_HANDLER("sem_open");
	}

	while(true)
	{
		sem_wait(sem_reader);
		++(*readers_count);
		if(*readers_count == 1)
		{
			sem_wait(sem_writer);
		}
		sem_post(sem_reader);
		
		task_execute();

		sem_wait(sem_reader);	
		--(*readers_count);
		if(*readers_count == 0)
		{
			sem_post(sem_writer);
		}
		sem_post(sem_reader);

		usleep(SLEEP_TIME_MICROSECONDS);
	}


	exit(EXIT_SUCCESS);
}

void task_execute()
{
	int result = 0;
	for(int i = 0; i < TAB_LENGTH; ++i)
	{
		if(shared_array[i] >= min && shared_array[i] <= max)
		{
			++result;
		}
	}
	printf("R%d: #%ld  numbers in [%d,%d]: %d\n", reader_id, tasks_counter, min, max, result);
	fflush(stdout);

	++tasks_counter;
}

void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	shm_unlink(READERS_COUNT);
	sem_unlink(READERS_SEMAPHORE);
}
