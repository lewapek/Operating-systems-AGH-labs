#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s id\n", name)

#include "header.h"
extern int shared_memory_size;

void execute_task();
void sigint_handler(int sig);
void atexit_function();

void *shared_memory;
int *current_task_number;
long *task_counter;
double *task_tab;
int sem_id;
int consumer_id;

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	consumer_id = atoi(argv[1]);

	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	key_t shm_key = ftok(".", 1);
	key_t sem_key = ftok(".", 2);
	if(shm_key == -1 || sem_key == -1)
	{
		ERROR_HANDLER("ftok");
	}

	int shm_id = shmget(shm_key, shared_memory_size, 0);
	if(shm_id == -1)
	{
		ERROR_HANDLER("shmget");
	}

	sem_id = semget(sem_key, 2, 0);
	if(sem_id == -1)
	{
		ERROR_HANDLER("semget");
	}

	shared_memory = shmat(shm_id, NULL, 0);
	if(task_tab == (void *)-1)
	{
		ERROR_HANDLER("shmat");
	}

	struct sembuf SEM_PRODUCER_UP;
	struct sembuf SEM_CONSUMER_DOWN;

	SEM_PRODUCER_UP.sem_flg = 0;
	SEM_PRODUCER_UP.sem_num = SEM_PRODUCER;
	SEM_PRODUCER_UP.sem_op = 1;
	SEM_CONSUMER_DOWN.sem_flg = 0;
	SEM_CONSUMER_DOWN.sem_num = SEM_CONSUMER;
	SEM_CONSUMER_DOWN.sem_op = -1;

	current_task_number = (int *)shared_memory + 1;
	task_counter = (long *)(current_task_number + 1) + 1;
	task_tab = (double *)(task_counter + 1);

	while(true)
	{
		if(semop(sem_id, &SEM_CONSUMER_DOWN, 1) == -1)
		{
			ERROR_HANDLER("semop");
		}

		execute_task();
		usleep(SLEEP_TIME_MICROSECONDS);
		
		if(semop(sem_id, &SEM_PRODUCER_UP, 1) == -1)
		{
			ERROR_HANDLER("semop");
		}
	}

	exit(EXIT_SUCCESS);
}

void execute_task()
{
	double max = task_tab[*current_task_number * TASK_LENGTH];
	double min = task_tab[*current_task_number * TASK_LENGTH];
	double avg = task_tab[*current_task_number * TASK_LENGTH];
	
	for(int i = 1; i < TASK_LENGTH; ++i)
	{
		double tmp_val = task_tab[*current_task_number * TASK_LENGTH + i];
		
		if(tmp_val > max)
		{
			max = tmp_val;
		}

		if(tmp_val < min)
		{
			min = tmp_val;
		}

		avg += tmp_val;//actually, it is sum, it will become average below
	}
	avg /= (double)TASK_LENGTH;
	
	printf("C%d: #%ld  task(%d): min=%.2lf  max=%.2lf  avg=%.2lf\n", consumer_id, *task_counter, *current_task_number, min, max, avg);
	fflush(stdout);

	++(*task_counter);
	*current_task_number = (*current_task_number + 1) % TASK_TAB_LENGTH;
}

void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	semctl(sem_id, 0, IPC_RMID);
	shmdt(shared_memory);
}
