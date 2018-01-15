#include <stdio.h>
#include <unistd.h>

int main()
{
	int i = 1;

	while(i == 1)
	{
		printf("Inside 'while' loop...\n");
		sleep(2);
	}

	printf("After 'while' loop.\n");

	return 0;
}