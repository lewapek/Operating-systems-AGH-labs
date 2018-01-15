#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>
#include "processes.h"

int counter;
int TICKS;
int n;

void show_print_time(struct tms *td, clock_t start_real_time, clock_t child_real_time, const char *sufix);
static void write_to_file(const char *name, int a, double b);

int main(int argc, char const *argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage:\n\t%s N prefix_name\n", argv[0]);
		_exit(-1);
	}

	struct tms t1, t2, td;

	n = atoi(argv[1]);
	counter = 0;
	TICKS = sysconf(_SC_CLK_TCK);

	clock_t start_real_time = times(&t1);
	clock_t child_real_time = 0;

	int i;
	for(i = 0; i < n; ++i)
	{
		child_real_time += processes_go();
	}

	start_real_time = times(&t2) - start_real_time;

	td.tms_stime = t2.tms_stime - t1.tms_stime;
	td.tms_utime = t2.tms_utime - t1.tms_utime;
	td.tms_cstime = t2.tms_cstime - t1.tms_cstime;
	td.tms_cutime = t2.tms_cutime - t1.tms_cutime;

	show_print_time(&td, start_real_time, child_real_time, argv[2]);

	printf("counter = %d\n", counter);

	return 0;
}

void show_print_time(struct tms *td, clock_t start_real_time, clock_t child_real_time, const char *sufix)
{
	fprintf(stderr, "type     system   user     sum      real\n");
	fprintf(stderr, "parent   %05.2lf    %05.2lf    %05.2lf    %05.2lf\n",
		(double)td->tms_stime / TICKS,
		(double)td->tms_utime / TICKS,
		(double)(td->tms_stime + td->tms_utime) / TICKS,
		(double)start_real_time / TICKS);
	fprintf(stderr, "child    %05.2lf    %05.2lf    %05.2lf    %05.2lf\n",
		(double)td->tms_cstime / TICKS,
		(double)td->tms_cutime / TICKS,
		(double)(td->tms_cstime + td->tms_cutime) / TICKS,
		(double)child_real_time / TICKS);
	fprintf(stderr, "total    %05.2lf    %05.2lf    %05.2lf    %05.2lf\n",
		(double)(td->tms_stime + td->tms_cstime) / TICKS,
		(double)(td->tms_utime + td->tms_cutime) / TICKS,
		(double)(td->tms_stime + td->tms_utime + td->tms_cstime + td->tms_cutime) / TICKS,
		(double)(start_real_time + child_real_time) / TICKS);

	char name[100];

	//parent times
	name[0] = '\0';
	strcat(name, "parent_sys_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)td->tms_stime / TICKS);

	name[0] = '\0';
	strcat(name, "parent_user_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)td->tms_utime / TICKS);

	name[0] = '\0';
	strcat(name, "parent_sum_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(td->tms_stime + td->tms_utime) / TICKS);

	name[0] = '\0';
	strcat(name, "parent_real_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)start_real_time / TICKS);

	//childrens times
	name[0] = '\0';
	strcat(name, "child_sys_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)td->tms_cstime / TICKS);

	name[0] = '\0';
	strcat(name, "child_user_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)td->tms_cutime / TICKS);

	name[0] = '\0';
	strcat(name, "child_sum_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(td->tms_cstime + td->tms_cutime) / TICKS);

	name[0] = '\0';
	strcat(name, "child_real_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)child_real_time / TICKS);

	//total times
	name[0] = '\0';
	strcat(name, "total_sys_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(td->tms_stime + td->tms_cstime) / TICKS);

	name[0] = '\0';
	strcat(name, "total_user_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(td->tms_utime + td->tms_cutime) / TICKS);

	name[0] = '\0';
	strcat(name, "total_sum_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(td->tms_stime + td->tms_utime + td->tms_cstime + td->tms_cutime) / TICKS);

	name[0] = '\0';
	strcat(name, "total_real_");
	strcat(name, sufix);
	strcat(name, ".points");
	write_to_file(name, n, (double)(start_real_time + child_real_time) / TICKS);
}

static void write_to_file(const char *name, int a, double b)
{
	FILE *fp = fopen(name, "a");
	if(fp == NULL)
	{
		fprintf(stderr, "Problem appending to file.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "%d %lf\n", a, b);
	fclose(fp);
}
