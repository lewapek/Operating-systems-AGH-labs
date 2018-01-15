#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_MSG_LENGTH 1024

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s fifo_name file_name\n", name);

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	size_t name_length, message_length;

	int fifo_file_descriptor = open(argv[1], O_WRONLY);
	if(fifo_file_descriptor < 0)
	{
		perror("fifo open failed");
		exit(EXIT_FAILURE);
	}

	char *msg = NULL;
	getline(&msg, &message_length, stdin);
	name_length = strlen(argv[2]);

	write(fifo_file_descriptor, &name_length, sizeof(size_t));
	write(fifo_file_descriptor, argv[2], name_length);
	write(fifo_file_descriptor, &message_length, sizeof(size_t));
	write(fifo_file_descriptor, msg, message_length);

	syncfs(fifo_file_descriptor);
	
	close(fifo_file_descriptor);

	return 0;
}
