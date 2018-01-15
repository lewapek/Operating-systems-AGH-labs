#include <stdio.h>
#include "dir_search.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s <path> <date> {<|=|>}\n", name)

int main(int argc, char **argv)
{
	if(argc != 4)
	{
		USAGE;
		return 1;
	}

	char operator = argv[3][0];
	if(operator != '<' && operator != '=' && operator != '>')
	{
		fprintf(stderr, "Bad operator.\n");
		USAGE;
		return 2;
	}

	dir_search(argv[1], argv[2], operator);
	
	return 0;
}
