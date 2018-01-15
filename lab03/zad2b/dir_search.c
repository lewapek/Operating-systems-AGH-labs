#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ftw.h>
#include <limits.h>

#include "dir_search.h"

struct tm *dir_search_date_to_check;
char dir_search_operator;

static struct tm *dir_search_parse_date(char *date);
static int dir_search_ftw_callback(const char *file, const struct stat *stat, int type);
static int dir_search_date_bigger(struct tm *date1, struct tm *date2);
static int dir_search_date_less(struct tm *date1, struct tm *date2);
static int dir_search_date_equal(struct tm *date1, struct tm *date2);
static int dir_search_compare_dates(struct tm *date1, struct tm *date2);


void dir_search(char *path, char *date, char op)
{
	dir_search_operator = op;
	dir_search_date_to_check = dir_search_parse_date(date);
	
	if(ftw(path, &dir_search_ftw_callback, FOPEN_MAX) != 0)
	{
		perror("Error: ftw command");
		free(dir_search_date_to_check);
		exit(1);
	}
}

static struct tm* dir_search_parse_date(char *date)
{
	struct tm *timer = (struct tm *)malloc(sizeof(struct tm));
	
	if(strptime(date, "%Y-%m-%d", timer) == 0)
	{
		fprintf(stderr, "Invalid date, got: %s\n", date);
		exit(2);
	}

	return timer;
}

static int dir_search_ftw_callback(const char *file, const struct stat *stat, int type)
{
	struct tm *modify_date = gmtime(&stat->st_mtime);
	
	if(dir_search_compare_dates(modify_date, dir_search_date_to_check) == 0)
	{
		printf("%s\n", file);
	}
	
	return 0;
}

static int dir_search_date_bigger(struct tm *date1, struct tm *date2)
{
	if(date1->tm_year > date2->tm_year)
	{
		return 0;
	}
	else if(date1->tm_year == date2->tm_year)
	{
		if(date1->tm_mon > date2->tm_mon)
		{
			return 0;
		}
		else if(date1->tm_mon == date2->tm_mon)
		{
			if(date1->tm_mday > date2->tm_mday)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int dir_search_date_less(struct tm *date1, struct tm *date2)
{
	if(date1->tm_year < date2->tm_year)
	{
		return 0;
	}
	else if(date1->tm_year == date2->tm_year)
	{
		if(date1->tm_mon < date2->tm_mon)
		{
			return 0;
		}
		else if(date1->tm_mon == date2->tm_mon)
		{
			if(date1->tm_mday < date2->tm_mday)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int dir_search_date_equal(struct tm *date1, struct tm *date2)
{
	if(date1->tm_year == date2->tm_year && date1->tm_mon == date2->tm_mon && date1->tm_mday == date2->tm_mday)
	{
		return 0;
	}

	return 1;
}

static int dir_search_compare_dates(struct tm *date1, struct tm *date2)
{
	if(dir_search_operator == '=')
	{
		return dir_search_date_equal(date1, date2);
	}	
	else if(dir_search_operator == '<')
	{
		return dir_search_date_less(date1, date2);
	}	
	else if(dir_search_operator == '>')
	{
		return dir_search_date_bigger(date1, date2);
	}
	
	return 1;
}
