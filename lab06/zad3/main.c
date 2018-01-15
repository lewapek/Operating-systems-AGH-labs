#define _GNU_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s\n", name);

int main(int argc, char const *argv[])
{
	if(argc != 1)
	{
		fprintf(stderr, "This program doesn't require arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	const char *command = "gzip - > message.gz";

	FILE *stream = popen(command, "w");
	if(stream == NULL)
	{
		perror("popen failed");
		exit(EXIT_FAILURE);
	}

	char *text;
	size_t text_length;

	while(-1 != getline(&text, &text_length, stdin))
	{
		fprintf(stream, "%s", text);
	}

	int status = pclose(stream);
	if(status == -1)
	{
		perror("pclose failed");
		exit(EXIT_FAILURE);
	}

	return 0;
}
