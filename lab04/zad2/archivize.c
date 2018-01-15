#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define ARCHIVE "archive/"
#define USAGE fprintf(stderr, "Usage:\n\t%s file period_time limit\n", argv[0])

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		USAGE;
		return -1;
	}

	char *file = argv[1];//filename
	int period = atoi(argv[2]);
	int limit = atoi(argv[3]);
	int last_time = 0;

	int fp = open(file, O_RDWR);

	if(fp < 0)
	{
		perror("Unable to open file");
		return -1;
	}

	struct stat fstats;

	bool condition = true;
	while(condition)
	{
		printf("archivize: 'while' begin...\n");
		fstat(fp, &fstats);

		if(fstats.st_size > limit && fstats.st_mtime != last_time)
		{
			pid_t pid = fork();

			if(pid < 0)
			{
				perror("Fork error");
				return -1;
			}

			if(pid == 0)
			{
				printf("archivize: Creating copy of file '%s'...\n", file);
				
				char *data = (char *)calloc(21, sizeof(char));

				if(!data)
				{
					return -1;
				}

				time_t t;
				struct tm *tmp;

				t = time(0);
				tmp = localtime(&t);

				strftime(data, 21 * sizeof(char), "_%Y-%m-%d_%H-%M-%S", tmp);

				char *dest_path = (char*)calloc(21 + strlen(file) + strlen(ARCHIVE), sizeof(char));

				strcpy(dest_path, ARCHIVE);
				strcat(dest_path, file);
				strcat(dest_path, data);

				free(data);

				execlp("cp", "cp", file, dest_path, 0);

				perror("Execlp error");
				
				return -1;

			}
			else
			{
				wait(0);
				ftruncate(fp, 0);
			}
		}

		last_time = fstats.st_mtime;
		sleep(period);
	}

	close(fp);
	
	return 0;
}
