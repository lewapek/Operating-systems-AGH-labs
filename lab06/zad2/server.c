#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s fifo_name\n", name);

static bool loop_condition = true;
static void sigint_handler(int s);

int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, sigint_handler);
	printf("To finish this program press Ctrl^C.\n");

	size_t name_length, message_length;
	char *name, *msg;

	int check = mkfifo(argv[1], 0666);
	if(check != 0)
	{
		perror("mkfifo failed");
		exit(EXIT_FAILURE);
	}

	int fifo_file_descriptor = open(argv[1], O_RDONLY);
	if(fifo_file_descriptor < 0)
	{
		perror("fifo open failed");
		exit(EXIT_FAILURE);
	}

	while(loop_condition)
	{
		check = read(fifo_file_descriptor, &name_length, sizeof(size_t));
		if(check == 0)
		{
			continue;
		}

		name = (char *)malloc(name_length);
		read(fifo_file_descriptor, name, name_length);
		read(fifo_file_descriptor, &message_length, sizeof(size_t));
		msg = (char *)malloc(message_length);
		read(fifo_file_descriptor, msg, message_length);
		
		FILE *file = fopen(name, "a");
		if(file == NULL)
		{
			free(msg);
			free(name);
			continue;
		}

		time_t timer = time(NULL);
		fprintf(file, "%s%s\n", ctime(&timer), msg);
		fclose(file);
		free(msg);
		free(name);
	}

	close(fifo_file_descriptor);
	unlink(argv[1]);

	return 0;
}

void sigint_handler(int s)
{
	loop_condition = false;
	printf("Signal SIGINT received. Program will finish soon.\n");
}
