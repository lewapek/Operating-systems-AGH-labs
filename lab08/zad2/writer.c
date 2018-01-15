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
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s id\n", name)

void task_execute();
void sigint_handler(int sig);
void atexit_function();

int *shared_array;
int shm_fd;
sem_t *sem_writer;

long modifications_counter = 1;

int writer_id;

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	writer_id = atoi(argv[1]);

	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	shm_fd = shm_open(SHARED_MEM, O_RDWR | O_CREAT, 0644);
	if(shm_fd == -1)
	{
		ERROR_HANDLER("shm_open");
	}

	if(ftruncate(shm_fd, shared_memory_size) == -1)
	{
		ERROR_HANDLER("ftruncate");
	}

	shared_array = (int *)mmap(NULL, shared_memory_size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(shared_array == MAP_FAILED)
	{
		ERROR_HANDLER("mmap");
	}

	sem_writer = sem_open(WRITERS_SEMAPHORE, O_CREAT, 0644, 1);
	if(sem_writer == SEM_FAILED)
	{
		ERROR_HANDLER("sem_open");
	}

	srand(time(NULL));
	while(true)
	{
		sem_wait(sem_writer);
		task_execute();
		sem_post(sem_writer);

		usleep(SLEEP_TIME_MICROSECONDS);
	}

	exit(EXIT_SUCCESS);
}

void task_execute()
{
	int iterations_number = rand() % TAB_LENGTH + 1;

	for(int i = 0; i < iterations_number; ++i)
	{
		int index = rand() % TAB_LENGTH;
		shared_array[index] = 10 - rand() % 21;
	}

	printf("W%d: array modified(%ld)\n", writer_id, modifications_counter);
	fflush(stdout);

	++modifications_counter;
}

void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	shm_unlink(SHARED_MEM);
	sem_unlink(WRITERS_SEMAPHORE);
}
