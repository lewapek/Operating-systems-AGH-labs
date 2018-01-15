#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ERROR { perror(__func__); exit(1); }

#define BUF_SIZE 1024
#define OFFSET (2* sizeof(int))

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s generate struct_size structs_quantity filename\n\t%s sort filename\n\n", name, name);

typedef struct structure_struct
{
	int key;
	char *data;
} structure;

void generate(int struct_size, int count, char *filename);
void sort(char *filename);
void time_measure();

int main(int argc, char** argv)
{
	if(argc == 5)
	{
		if (strcmp(argv[1], "generate") != 0 && strcmp(argv[1], "generuj") != 0)
		{
			USAGE;
			exit(1);
		}

		int struct_size = atoi(argv[2]);
		int count = atoi(argv[3]);
		char *filename = malloc (strlen(argv[4]));
		strcpy(filename, argv[4]);

		generate(struct_size, count, filename);		
	}
	else if(argc == 3)
	{
		if(strcmp(argv[1], "sort") != 0 && strcmp(argv[1], "sortuj") != 0)
		{
			USAGE;
			exit(2);//finishes program
		}

		char *filename = malloc(strlen(argv[2]));
		if(filename == NULL)
		{
			perror("Malloc error");
			exit(1);
		}

		strcpy(filename, argv[2]);
			
		char *filename_copy = malloc(strlen(filename) + 5);
		if(filename_copy == NULL)
		{
			perror("Malloc error");
			exit(1);
		}

		strcpy(filename_copy, "copy_");
		strcpy(&(filename_copy[5]), filename);
#ifdef SYS				
		//file copy
		int original = open(filename, O_RDONLY);
		int copy = open(filename_copy, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#else
		//file copy
		FILE* original = fopen(filename, "r");
		FILE* copy = fopen(filename_copy, "w");
#endif		
		int readed = 0;
		char buffer[BUF_SIZE];
		
		do
		{
#ifdef SYS
			readed = read(original, buffer, sizeof(char) * BUF_SIZE);
			write(copy, buffer, sizeof(char) * readed);
#else
			readed = fread(buffer, sizeof(char), BUF_SIZE, original);
			fwrite(buffer, sizeof(char), readed, copy);
#endif
		} while(readed > 0);
#ifdef SYS
		close(original);
		close(copy);
#else
		fclose(original);
		fclose(copy);
#endif
		time_measure();
		sort(filename);
		time_measure();
	}
	else
	{
		fprintf(stderr, "Wrong number of arguments!\n");
		USAGE;
		exit(3);
	}
	
	return 0;
}

void time_measure()
{
	static int count = 1;
	static long clk;
	clk = sysconf (_SC_CLK_TCK);
	struct tms start = {0, 0, 0, 0};
	struct tms act = {0, 0, 0, 0};
	struct tms prev = {0, 0, 0, 0};
	
	if (count == 1)
	{
		errno = 0;
		times (&start);
		perror("time_measure()");
		prev = start;
	}
	else
	{
		errno = 0;
		times (&act);
		
		perror ("time_measure()");
		printf("-------------------------------------\n");
		printf("Time measurement\nFrom the beginning:\n");
		printf("\tUser time:   %.2f\n", (double)(act.tms_utime - start.tms_utime) / clk);
		printf("\tSystem time: %.2f\n", (double)(act.tms_stime - start.tms_stime) / clk);
		printf("From last measurement:\n");
		printf("\tUser time:   %.2f\n", (double)(act.tms_utime - prev.tms_utime) / clk);
		printf("\tSystem time: %.2f\n", (double)(act.tms_stime - prev.tms_stime) / clk);
		printf("-------------------------------------\n");
		
		prev = act;
	}

	++count; 
	
	return;
}

void generate(int struct_size, int count, char* filename)
{
#ifdef SYS
	int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (file == -1)
	{
		ERROR;
	}
	
	//header save
	write(file, &struct_size, sizeof(int) * 1);
	write(file, &count, sizeof(int) * 1);
#else
	FILE* file = fopen(filename, "w");
	if (!file)
	{
		ERROR;
	}
	
	//header save
	fwrite(&struct_size, sizeof(int), 1, file);
	fwrite(&count, sizeof(int), 1, file);
#endif
	
	int data_size = struct_size - sizeof(int);
	int i;
	for (i = 0; i < count; ++i)
	{
		//structure alloc
		structure *ts = malloc(sizeof(structure));
		
		ts->data = malloc(data_size);
		
		//data create
		ts->key = rand() % INT_MAX;
		int j;
		for(j = 0; j<data_size; ++j)
		{
			(ts->data)[j] = rand() % INT_MAX;
		}

#ifdef SYS
		//write to file
		write(file, &(ts->key), sizeof(int) * 1);
		write(file, ts->data, sizeof(char) * data_size);
#else
		//write to file
		fwrite(&(ts->key), sizeof(int), 1, file);
		fwrite(ts->data, sizeof(char),  data_size, file);
#endif		
		//dealokacja struktury
		free (ts->data);
		free (ts);
	}

#ifdef SYS
	close(file);
#else
	fclose(file);
#endif

	return;
}

void sort(char* filename)
{
#ifdef SYS
	int file = open(filename, O_RDWR);
	if(file == -1)
	{
		ERROR;
	}
#else
	FILE* file = fopen(filename, "r+");
	if(file == NULL)
	{
		ERROR;
	}
#endif

	int struct_size;
	int count;

#ifdef SYS
	read(file, &struct_size, sizeof(int));
	read(file, &count, sizeof(int));
#else
	fread(&struct_size, sizeof(int), 1, file);
	fread(&count, sizeof(int), 1, file);
#endif
	
	int data_size = struct_size - sizeof (int);
	int i;
	for (i = 0; i < count; ++i)
	{
		int j;
		for (j = i; j < count; ++j)
		{
			structure* ts1 = malloc(sizeof(structure));
			structure* ts2 = malloc(sizeof(structure));
		
			ts1->data = malloc(data_size);
			ts2->data = malloc(data_size);
			
#ifdef SYS
			//data read
			lseek(file, i * struct_size + OFFSET, SEEK_SET);
			read(file, &(ts1->key), sizeof(int) * 1);
			read(file, ts1->data, sizeof(char) * data_size);
			
			lseek(file, j * struct_size + OFFSET, SEEK_SET);
			read(file, &(ts2->key), sizeof(int) * 1);
			read(file, ts2->data, sizeof(char) * data_size);
#else
			//date read
			fseek(file, i * struct_size + OFFSET, SEEK_SET);
			fread(&(ts1->key), sizeof(int), 1, file);
			fread(ts1->data, sizeof(char), data_size, file);
			
			fseek(file, j * struct_size + OFFSET, SEEK_SET);
			fread(&(ts2->key), sizeof(int), 1, file);
			fread(ts2->data, sizeof(char), data_size, file);
#endif

			//swap
			if (ts1->key/*i*/ > ts2->key/*j*/)
			{
#ifdef SYS
				//write to file
				lseek(file, j * struct_size + OFFSET, SEEK_SET);
				write(file, &(ts1->key), sizeof(int) * 1);
				write(file, ts1->data, sizeof(char) * data_size);
				
				lseek(file, i * struct_size + OFFSET, SEEK_SET);
				write(file, &(ts2->key), sizeof(int) * 1);
				write(file, ts2->data, sizeof(char) * data_size);
#else
				//write to file
				fseek(file, j * struct_size + OFFSET, SEEK_SET);
				fwrite(&(ts1->key), sizeof(int), 1, file);
				fwrite(ts1->data, sizeof(char), data_size, file);
				
				fseek(file, i * struct_size + OFFSET, SEEK_SET);
				fwrite(&(ts2->key), sizeof(int), 1, file);
				fwrite(ts2->data, sizeof(char), data_size, file);
#endif
			}
			
			//structure dealloc
			free (ts1->data);
			free (ts1);
			free (ts2->data);
			free (ts2);		
		}	
	}
	
#ifdef SYS
	close (file);
#else
	fclose(file);
#endif

	return;
}
