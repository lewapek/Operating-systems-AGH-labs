#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s\n", name);

static void signal_usr1_sigaction(int sig, siginfo_t *siginfo, void *ucontext);
static void set_sigaction();

static int finish_condition = 0;

int main(int argc, char const *argv[])
{
	if(argc > 1)
	{
		fprintf(stderr, "This program doesn't require arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	set_sigaction();

	while(finish_condition == 0)
	{
		continue;
	}

	return 0;
}

static void signal_usr1_sigaction(int sig, siginfo_t *siginfo, void *ucontext)
{
	clock_t utime = siginfo->si_utime;
	uid_t uid = siginfo->si_uid;
	pid_t pid = siginfo->si_pid;

	int additional_value;
	if(siginfo->si_code == SI_QUEUE)
	{
		additional_value = siginfo->si_int;
	}
	else
	{
		additional_value = 1;
	}

	int i;
	for(i = 0; i < additional_value; ++i)
	{
		printf("UID: %d, PID: %d, user time: %lld\n", (int)uid, (int)pid, (long long)utime);
	}

	//changing finish_condition to leave 'while' loop
	finish_condition = 1;
}

static void set_sigaction()
{
	struct sigaction sigusr1;
	sigusr1.sa_flags = SA_SIGINFO;
	sigusr1.sa_sigaction = signal_usr1_sigaction;

	int check = sigaction(SIGUSR1, &sigusr1, NULL);
	if(check != 0)
	{
		perror("Problem with setting sigaction");
		exit(EXIT_FAILURE);
	}
}
