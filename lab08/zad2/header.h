#define TAB_LENGTH 50
#define SLEEP_TIME_MICROSECONDS 500000
#define SHARED_MEM "/shared_mem"
#define READERS_COUNT "/readers_count"
#define READERS_SEMAPHORE "/readers_semaphore"
#define WRITERS_SEMAPHORE "/writers_semaphore"

#define ERROR_HANDLER(s) { perror(s); exit(EXIT_FAILURE); }

int shared_memory_size = TAB_LENGTH * sizeof(int);
