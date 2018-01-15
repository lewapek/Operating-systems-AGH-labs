#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>

#define USAGE fprintf(stderr, "Usage:\n\t%s file period_time bytes\n", argv[0])

int main(int argc, char const **argv)
{
	if(argc != 4)
	{
		USAGE;
		return -1;
	}

	FILE *file = fopen(argv[1], "a");

	if(file == NULL)
	{
		perror("Unable to open file");
		return -1;
	}

	bool condition = true;
	int period = atoi(argv[2]);
	int bytes = atoi(argv[3]);

	srand(time(NULL));

	while(condition)
	{
		char *data = (char *)calloc(20, sizeof(char));
		char *bytestr = (char *)malloc((bytes + 1) * sizeof(char));

		int i;
		for(i = 0; i < bytes; ++i)
		{
			bytestr[i] = 1 + rand() % 255;
		}

		bytestr[bytes] = '\0';

		time_t t;
		struct tm *tms;

		t = time(0);
		tms = localtime(&t);
		strftime(data, 20, "%Y-%m-%d %H:%M:%S", tms);

		fprintf(file, "%d %s %s\n", getpid(), data, bytestr);
		fflush(file);

		free(data);
		free(bytestr);

		sleep(period);
	}

	fclose(file);

	return 0;
}
