#include <stdio.h>

long fibonacci(int n)
{
	if(n <= 2)
	{
		return 1;
	}

	return fibonacci(n - 1) + fibonacci(n - 2);
}

int main()
{
	long result = fibonacci(7);

	printf("%ld\n", result);

	return 0;
}