#define TASK_LENGTH 50
#define TASK_TAB_LENGTH 10
#define SEM_PRODUCER 0
#define SEM_CONSUMER 1
#define SLEEP_TIME_MICROSECONDS 400000

#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

int shared_memory_size = 2 * sizeof(int) + 2 * sizeof(long) + TASK_TAB_LENGTH * TASK_LENGTH * sizeof(double);
