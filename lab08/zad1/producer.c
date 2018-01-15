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
#include <time.h>
#include <unistd.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s id\n", name)

#include "header.h"
extern int shared_memory_size;

union sem_union
{
	int val;/* Value for SETVAL */
	struct semid_ds *buffer;/* Buffer for IPC_STAT, IPC_SET */
	unsigned short *array;/* Array for GETALL, SETALL */
	struct seminfo *sem_info;/* Buffer for IPC_INFO */
};

void put_task();
void sigint_handler(int sig);
void atexit_function();

void *shared_memory;
int *curren_task_num;
long *task_counter;
double *task_tab;
bool is_sem_created = false;
int shm_id;
int sem_id;
int producer_id;

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	producer_id = atoi(argv[1]);

	atexit(atexit_function);
	signal(SIGINT, sigint_handler);

	key_t shm_key = ftok(".", 1);
	key_t sem_key = ftok(".", 2);
	if(shm_key == -1 || sem_key == -1)
	{
		ERROR_HANDLER("ftok");
	}

	shm_id = shmget(shm_key, shared_memory_size, IPC_CREAT | 0600);
	if(shm_id == -1)
	{
		ERROR_HANDLER("shmget");
	}

	sem_id = semget(sem_key, 2, IPC_CREAT | IPC_EXCL | 0600);
	if(sem_id == -1)
	{
		if(errno == EEXIST)
		{
			is_sem_created = true;
		}
		else
		{
			ERROR_HANDLER("semget");
		}
	}

	if(is_sem_created == true)
	{
		sem_id = semget(sem_key, 2, 0);
		if(sem_id == -1)
		{
			ERROR_HANDLER("semget");
		}
	}

	shared_memory =  shmat(shm_id, NULL, 0);
	if(shared_memory == (void *) -1)
	{
		ERROR_HANDLER("shmat");
	}

	struct sembuf SEM_PRODUCER_DOWN;
	struct sembuf SEM_CONSUMER_UP;

	SEM_PRODUCER_DOWN.sem_flg = 0;
	SEM_PRODUCER_DOWN.sem_num = SEM_PRODUCER;
	SEM_PRODUCER_DOWN.sem_op = -1;
	SEM_CONSUMER_UP.sem_flg = 0;
	SEM_CONSUMER_UP.sem_num = SEM_CONSUMER;
	SEM_CONSUMER_UP.sem_op = 1;

	curren_task_num = (int *)shared_memory;
	task_counter = (long *)((int *)shared_memory + 2);
	task_tab = (double *)(task_counter + 2);

	union sem_union arg;
	if(!is_sem_created)
	{
		curren_task_num[0] = 0;//actual producer's task
		curren_task_num[1] = 0;//actual consumer's task
		task_counter[0] = 0;//producer's completed tasks
		task_counter[1] = 0;//consumer's completed tasks

		arg.val = TASK_TAB_LENGTH;
		if(semctl(sem_id, SEM_PRODUCER, SETVAL, arg) == -1)
		{
			ERROR_HANDLER("semctl");
		}
	
		arg.val = 0;
		if(semctl(sem_id, SEM_CONSUMER, SETVAL, arg) == -1)
		{
			ERROR_HANDLER("semctl");
		}
	}

	srand(time(NULL));
	while(true)
	{
		if(semop(sem_id, &SEM_PRODUCER_DOWN, 1) == -1)
		{
			ERROR_HANDLER("semop");
		}

		put_task();
		usleep(SLEEP_TIME_MICROSECONDS);
		
		if(semop(sem_id, &SEM_CONSUMER_UP, 1) == -1)
		{
			ERROR_HANDLER("semop");
		}
	}


	exit(EXIT_SUCCESS);
}

void put_task()
{
	for(int i = 0; i < TASK_LENGTH; ++i)
	{
		task_tab[*curren_task_num * TASK_LENGTH + i] = ((double)rand() / RAND_MAX) * (rand() % 100);
	}

	printf("P%d: #%ld  task put at %d\n", producer_id, *task_counter, *curren_task_num);
	fflush(stdout);

	++(*task_counter);
	*curren_task_num = (*curren_task_num + 1) % TASK_TAB_LENGTH;
}

void sigint_handler(int sig)
{
	exit(EXIT_SUCCESS);
}

void atexit_function()
{
	if(!is_sem_created)
	{
		shmctl(shm_id, IPC_RMID, NULL);
		semctl(sem_id, 0, IPC_RMID);
	}
	shmdt(shared_memory);
}
