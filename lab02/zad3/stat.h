#ifndef _STAT_H_
#define _STAT_H_

#include <stdio.h>
#include <time.h>
#include <mem_management/mem_management.h>

typedef struct timers_struct
{
	long usersec, userusec;
	long systemsec, systemusec;
	time_t realsec;
	long realnsec;
} timers;

mem_diagnostic_info (*m_diagnose)();
void (*m_print_diagnostic_info)(FILE *, mem_diagnostic_info);

void stat_init();
void stat_show(FILE *fp);

#endif /*_STAT_H_*/
