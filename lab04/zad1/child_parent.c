#include "child_parent.h"

int child(void *arg)
{
	++counter;

	return 0;
}

int parent()
{
	int status;
	waitpid(-1, &status, 0);
	
	return WEXITSTATUS(status);
}