#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/types.h>

#include "processes.h"
#include "child_parent.h"

clock_t processes_go()
{
	pid_t pid;
	clock_t start, end;

	start = times(0);

	pid = vfork();

	if(pid < 0)
	{
		perror("Fork error");
		_exit(-1);
	}

	if(pid == 0)
	{
		_exit(child(0));
	}
	else
	{
		parent();
		end = times(0);
	}

	return end - start;
}
