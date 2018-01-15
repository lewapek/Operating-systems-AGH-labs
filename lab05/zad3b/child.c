#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s\n", name);

#define MAX_CHILD_NAME 50

static void signal_usr1_handler(int sig);
static void signal_usr2_handler(int sig);
static void set_sigaction();
static inline void sigaction_check(int check);

static bool set_child_program_exec_name(const char *name, char *exec_name, int max_length);

static inline void check_send_by_kill();

static int received_signals = 0;
static bool finish_condition = false;
static pid_t ppid;

int main(int argc, char const *argv[])
{
	if(argc > 1)
	{
		fprintf(stderr, "This program doesn't require arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	ppid = getppid();

	set_sigaction();
	
	char child_exec_name[MAX_CHILD_NAME];
	set_child_program_exec_name(argv[2], child_exec_name, MAX_CHILD_NAME);

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	while(!finish_condition)
	{
		pause();
	}

	int check = kill(ppid, SIGUSR2);
	check_send_by_kill(check);

	return 0;
}

static void signal_usr1_handler(int sig)
{
	++received_signals;
	printf("  child: SIGUSR1(%d) received\n", received_signals);

	int check = kill(ppid, SIGUSR1);
	check_send_by_kill(check);
}

static void signal_usr2_handler(int sig)
{
	finish_condition = true;
	printf("  child: SIGUSR2 received\n");
}

static void set_sigaction()
{
	int check;
	struct sigaction sigusr1, sigusr2;
	
	sigusr1.sa_handler = signal_usr1_handler;
	sigusr2.sa_handler = signal_usr2_handler;

	sigfillset(&(sigusr1.sa_mask));
	sigfillset(&(sigusr2.sa_mask));

	check = sigaction(SIGUSR1, &sigusr1, NULL);
	sigaction_check(check);
	check = sigaction(SIGUSR2, &sigusr2, NULL);
	sigaction_check(check);
}

static inline void sigaction_check(int check)
{
	if(check != 0)
	{
		perror("Problem with setting sigaction");
		exit(EXIT_FAILURE);
	}
}

static bool set_child_program_exec_name(const char *name, char *exec_name, int max_length)
{
	if(2 + strlen(name) >= max_length)
	{
		return false;
	}

	strcpy(exec_name, "./");
	strcat(exec_name, name);

	return true;
}

static inline void check_send_by_kill(int is_send)
{
	if(is_send != 0)
	{
		perror("Problem with sending SIGUSR1 using kill");
		exit(EXIT_FAILURE);
	}
}
