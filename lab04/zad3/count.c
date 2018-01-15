#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>

#define USAGE fprintf(stderr, "Usage:\n\t%s [-v] [-w]\n", argv[0])
#define SLEEP_TIME 30

static bool is_verbose = false;
static bool is_wait = false;
static char **arguments;
static char *extensions;

static int walk(char *path);
static bool matches(char *filename);

int main(int argc, char **argv)
{
	if(argc > 3)
	{
		USAGE;
		return -1;
	}

	int i;
	for(i = 1; i < argc; ++i)
	{
		if(strcmp(argv[i], "-v") == 0)
		{
			is_verbose = true;
		}

		if(strcmp(argv[i], "-w") == 0)
		{
			is_wait = true;
		}
	}

	arguments = argv;

	char *path = getenv("PATH_TO_BROWSE");

	if(path == NULL)
	{
		path = ".";//current directory
	}

	extensions = getenv("EXT_TO_BROWSE");

	int counter = walk(path);

	printf("counter = %d\n", counter);
	fflush(stdout);

	return counter < 0 ? 0: counter;
}

static int walk(char *path){
	int counter = 0;
	int status;

	DIR *dir = opendir(path);
	struct dirent *entry;
	struct stat filestat;
	char *filepath;
	pid_t pid;

	if(dir == NULL)
	{
		perror("Error opening directory");
		return -1;
	}

	if(is_verbose)
	{
		printf("%s\n", path);
		fflush(stdout);
	}

	while((entry = readdir(dir)) != NULL)
	{
		//ommitting '.' and '..'
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
			continue;
		}

		filepath = (char *)malloc((strlen(path) + strlen(entry->d_name) + 2) * sizeof(char));
		strcpy(filepath, path);
		strcat(filepath, "/");
		strcat(filepath, entry->d_name);

		if(stat(filepath, &filestat) < 0)
		{
			perror("Stat error");
			return -1;
		}

		if(S_ISDIR(filestat.st_mode))
		{
			pid = fork();
			if(pid < 0)
			{
				perror("Fork error");
				return -1;
			}
			if(pid == 0)
			{
				if (setenv("PATH_TO_BROWSE", filepath, 1) == -1)
				{
					perror("Setenv error");
					return -1;
				}

				if (execvp(arguments[0], arguments) == -1)
				{
					perror("Execvp error");
					return -1;
				}
			}
		}
		else if(S_ISREG(filestat.st_mode) && matches(filepath))
		{
			++counter;
		}

		free(filepath);
	}

	closedir(dir);

	if (is_wait)
	{
		sleep(SLEEP_TIME);
	}

	errno = 0;
	do
	{
		if(wait(&status) != -1)
		{
			counter += WEXITSTATUS(status);
		}
	}

	while(errno == 0);

	if (errno != ECHILD)
	{
		perror("Wait error");
		return -1;
	}

	return counter;
}

static bool matches(char *filename)
{
	if(extensions == 0 || strcmp("", extensions) == 0)
	{
		return true;
	}

	char *exts = (char *)malloc((strlen(extensions) + 1) * sizeof(char));
	strcpy(exts, extensions);

	char *extensions = strtok(exts, ":");
	int extlen = 0;
	int filelen = strlen(filename);

	if(extensions == 0)
	{
		extlen = strlen(exts);
		if(filelen >= extlen && strcmp(exts, filename + filelen - extlen) == 0)
		{
			free(exts);
			return true;
		}
	}
	else
	{
		while(extensions != NULL)
		{
			extlen = strlen(extensions);
			if(filelen >= extlen && strcmp(extensions, filename + filelen - extlen) == 0)
			{
				free(exts);
				return true;
			}
			
			extensions = strtok(0, ":");
		}
	}

	free(exts);

	return false;
}
