#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s input_filename output_filename\n", name);

static void pipe_execute(int pipe[], const char *input_name, const char *output_name);

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	int file_descriptor[2];

	pipe(file_descriptor);

	pid_t fork_return = fork();
	if(fork_return == -1)
	{
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	else if(fork_return == 0)
	{
		pipe_execute(file_descriptor, argv[1], argv[2]);
		exit(EXIT_SUCCESS);
	}
	else
	{
		pid_t pid;
		int status;

		while((pid = wait(&status)) != -1)
		{
			fprintf(stderr, "Process %d exited with status %d.\n", pid, WEXITSTATUS(status));
		}
	}
	
	return 0;
}

static void pipe_execute(int pipe[], const char *input_name, const char *output_name)
{
	char *command_gzip[] = {"gzip", "-9", "-", NULL};
	char *command_base64[] = {"base64", NULL};

	int input, output;
	
	pid_t fork_return = fork();
	if(fork_return == -1)
	{
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	else if(fork_return == 0)
	{
		dup2(pipe[0], 0);
		close(pipe[1]);

		output = open(output_name, O_WRONLY | O_CREAT, 0666);
		if(output < 0)
		{
			fprintf(stderr, "Unable to open output file '%s'.\n", output_name);
			exit(EXIT_FAILURE);
		}

		dup2(output, STDOUT_FILENO);
		close(output);

		execvp(command_base64[0], command_base64);

		perror(command_base64[0]);
		exit(EXIT_FAILURE);
	}
	else
	{
		dup2(pipe[1], 1);
		close(pipe[0]);
		
		input = open(input_name, O_RDONLY);
		if(input < 0)
		{
			fprintf(stderr, "Unable to open input file '%s'.\n", input_name);
			exit(EXIT_FAILURE);
		}

		dup2(input, STDIN_FILENO);
		close(input);
		
		execvp(command_gzip[0], command_gzip);

		perror(command_gzip[0]);
		exit(EXIT_FAILURE);
	}
}
