#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s text max\n", name);

#ifndef DELAY_SECONDS
#define DELAY_SECONDS 2
#endif /*DELAY_SECONDS*/

static void set_handlers_using_sigaction();
static inline void check_sigaction_handler(int val);

static void signal_tstp_handler(int sig);
static void signal_interrupt_handler(int sig);

static void print_text(const char *text);
static inline void print_normal(const char *text);
static inline void print_reverse(const char *text);
static inline void print_separator(char c, int lenght);

static int sigtstp_counter = 0;
static int max_sigtstp_counter;

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	max_sigtstp_counter = atoi(argv[2]);
	if(max_sigtstp_counter <= 0)
	{
		fprintf(stderr, "Argument 'max' must be positive integer.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	set_handlers_using_sigaction();

	while(1)
	{
		print_text(argv[1]);
		sleep(DELAY_SECONDS);
	}

	return 0;
}

static void set_handlers_using_sigaction()
{
	int check;

	struct sigaction sigint_sigaction_struct, sigtstp_sigaction_struct;
	sigint_sigaction_struct.sa_handler = signal_interrupt_handler;
	sigtstp_sigaction_struct.sa_handler = signal_tstp_handler;

	check = sigaction(SIGINT, &sigint_sigaction_struct, NULL);
	check_sigaction_handler(check);

	check = sigaction(SIGTSTP, &sigtstp_sigaction_struct, NULL);
	check_sigaction_handler(check);
}

static inline void check_sigaction_handler(int val)
{
	if(val != 0)
	{
		fprintf(stderr, "Error with setting singal handler.\n");
		exit(EXIT_FAILURE);
	}
}

static void signal_tstp_handler(int sig)
{
	int prev = sigtstp_counter;
	sigtstp_counter = (sigtstp_counter + 1) % (max_sigtstp_counter + 1);

	printf(" sigtstp_counter: %d -> %d\n", prev, sigtstp_counter);
}

static void signal_interrupt_handler(int sig)
{
	fprintf(stderr, "\nSignal %d (SIGINT) received. Exiting program.\n", sig);
	exit(EXIT_SUCCESS);
}

static void print_text(const char *text)
{
	int quantity = (sigtstp_counter + 2) / 2;
	int text_length = strlen(text);
	int i;

	print_separator('-', text_length);

	if(sigtstp_counter % 2 == 0)
	{
		for(i = 0; i < quantity; ++i)
		{
			print_normal(text);
		}
	}
	else
	{
		for(i = 0; i < quantity; ++i)
		{
			print_reverse(text);
		}
	}

	print_separator('-', text_length);
}

static inline void print_normal(const char *text)
{
	printf("%s\n", text);
	fflush(stdout);
}

static inline void print_reverse(const char *text)
{
	int i;
	for(i = strlen(text) - 1; i >= 0; --i)
	{
		putchar(text[i]);
	}

	putchar('\n');
	fflush(stdout);
}

static inline void print_separator(char c, int lenght)
{
	int i;
	for(i = 0; i < lenght; ++i)
	{
		putchar(c);
	}

	putchar('\n');
	fflush(stdout);
}
