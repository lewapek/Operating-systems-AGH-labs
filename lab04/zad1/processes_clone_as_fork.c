#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <sys/times.h>
#include <unistd.h>

#include "processes.h"
#include "child_parent.h"

clock_t processes_go()
{
	clock_t start, end;

    char *child_stack = (char *)malloc(STACK_SIZE);
    char *child_stackp;
    
    child_stackp = child_stack + STACK_SIZE;
	
	start = times(0);

	if(clone(&child, child_stackp, SIGCHLD, 0) < 0)
	{
		perror("Clone fail");
		_exit(-1);
	}

	parent();

	free(child_stack);

	end = times(0);

	return end - start;
}
