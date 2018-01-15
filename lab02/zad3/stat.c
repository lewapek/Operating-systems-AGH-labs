#include "stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <mem_management/mem_management.h>

#ifdef DYNAMIC
#include <dlfcn.h>
#endif

timers stat_start_time, stat_prev_time, stat_current_time;
unsigned int stat_counter;

void stat_init()
{
#ifdef DYNAMIC
	void *handle_mem = dlopen("libmem_management.so", RTLD_NOW);
	if(handle_mem == NULL)
	{
		fprintf(stderr, "%s\n", "FATAL ERROR");
		exit(1);
	}

	m_diagnose = dlsym(handle_mem, "mem_diagnose");
	m_print_diagnostic_info = dlsym(handle_mem, "mem_print_diagnostic_info");
	// dlclose(handle_mem);
#else
	m_diagnose = mem_diagnose;
	m_print_diagnostic_info = mem_print_diagnostic_info;
#endif

	struct timespec start;
	struct rusage usage;
	clock_gettime(CLOCK_REALTIME, &start);
	getrusage(RUSAGE_SELF, &usage);
	stat_start_time.realsec = start.tv_sec;
	stat_start_time.realnsec = start.tv_nsec;
	stat_start_time.usersec = usage.ru_utime.tv_sec;
	stat_start_time.userusec = usage.ru_utime.tv_usec;
	stat_start_time.systemsec = usage.ru_stime.tv_sec;
	stat_start_time.systemusec = usage.ru_stime.tv_usec;
	stat_prev_time = stat_start_time;

	stat_counter = 0;
}

void stat_show(FILE *fp)
{
	++stat_counter;

	struct timespec end;
	clock_gettime(CLOCK_REALTIME, &end);
	struct rusage usage;
	getrusage(RUSAGE_SELF, &usage);	
	stat_current_time.realsec = end.tv_sec;
	stat_current_time.realnsec = end.tv_nsec;
	stat_current_time.usersec = usage.ru_utime.tv_sec;
	stat_current_time.userusec = usage.ru_utime.tv_usec;
	stat_current_time.systemsec = usage.ru_stime.tv_sec;
	stat_current_time.systemusec = usage.ru_stime.tv_usec;

	if(fp != NULL)
	{
		fprintf(fp, "\n\n#############################################\n");
		fprintf(fp, "            DIAGNOSTIC POINT %04d\n", stat_counter);
		fprintf(fp, "=============================================\n");
		fprintf(fp, "                  From start\n");
		fprintf(fp, "=============================================\n");
		fprintf(fp, "Real:   %lf\nUser:   %lf\nSystem: %lf\n",
			stat_current_time.realsec - stat_start_time.realsec + (stat_current_time.realnsec - stat_start_time.realnsec) / 1000000000.0,
			stat_current_time.usersec - stat_start_time.usersec + (stat_current_time.userusec - stat_start_time.userusec) / 1000000.0,
			stat_current_time.systemsec - stat_start_time.systemsec + (stat_current_time.systemusec - stat_start_time.systemusec) / 1000000.0);
		fprintf(fp, "=============================================\n");
		fprintf(fp, "            From last measurement\n");
		fprintf(fp, "=============================================\n");
		fprintf(fp, "Real:   %lf\nUser:   %lf\nSystem: %lf\n",
			stat_current_time.realsec - stat_prev_time.realsec + (stat_current_time.realnsec - stat_prev_time.realnsec) / 1000000000.0,
			stat_current_time.usersec - stat_prev_time.usersec + (stat_current_time.userusec - stat_prev_time.userusec) / 1000000.0,
			stat_current_time.systemsec - stat_prev_time.systemsec + (stat_current_time.systemusec - stat_prev_time.systemusec) / 1000000.0);
		fprintf(fp, "=============================================\n");
		fprintf(fp, "               Diagnostic info\n");
		fprintf(fp, "=============================================\n");
		(*m_print_diagnostic_info)(fp, (*m_diagnose)());
		fprintf(fp, "#############################################\n\n");
	}
	
	stat_prev_time = stat_current_time;
}
