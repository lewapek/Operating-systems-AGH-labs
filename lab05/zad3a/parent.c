#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s quantity child_program_name\n", name);

#define MAX_CHILD_NAME 50

static void signal_usr1_handler(int sig);
static void signal_usr2_handler(int sig);
static void set_sigaction();
static inline void sigaction_check(int check);

static bool set_child_program_exec_name(const char *name, char *exec_name, int max_length);

static inline void check_send_by_kill();

static int received_signals = 0;
static bool finish_condition = false;

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	int quantity = atoi(argv[1]);
	if(quantity <= 0)
	{
		fprintf(stderr, "Argument 'quantity' must be positive integer.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	set_sigaction();
	
	char child_exec_name[MAX_CHILD_NAME];
	set_child_program_exec_name(argv[2], child_exec_name, MAX_CHILD_NAME);

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	pid_t pid = fork();
	if(pid == -1)
	{
		perror("Problem with fork");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		int exec_success = execl(child_exec_name, argv[2], NULL);
		if(exec_success == -1)
		{
			perror("Problem with execl() child program");
			exit(EXIT_FAILURE);
		}
	}

	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	printf("parent: sending %d SIGURS1 signals to child program '%s' (pid: %d).\n", quantity, argv[2], (int)pid);

	int i, is_send;
	for(i = 0; i < quantity; ++i)
	{
		is_send = kill(pid, SIGUSR1);
		check_send_by_kill(is_send);
	}

	printf("parent: sending SIGUSR2 signal to child program '%s' (pid: %d).\n", argv[2], (int)pid);
	
	is_send = kill(pid, SIGUSR2);
	check_send_by_kill(is_send);

	while(!finish_condition)
	{
		pause();
	}

	printf("parent: expected %d SIGUSR1 signals from child. Received %d.\n", quantity, received_signals);

	return 0;
}

static void signal_usr1_handler(int sig)
{
	++received_signals;
	printf("  parent: SIGUSR1(%d) received\n", received_signals);
}

static void signal_usr2_handler(int sig)
{
	finish_condition = true;
	printf("  parent: SIGUSR2 received\n");
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
