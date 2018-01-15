#include <stdio.h>
#define OUTER_LIMES 100000
#define INNER_LIMES 5000

int main()
{
	long i, j;
	double a = 1.0, b = 1.0;

	for(i = 0; i < OUTER_LIMES; ++i)
	{
		for(j = 0; j < INNER_LIMES; ++j)
		{
			a = 2 * b - 1.33;
		}
		
		for(j = 0; j < INNER_LIMES; ++j)
		{
			b = 0.5 * a - 0.45;
		}
		
		for(j = 0; j < INNER_LIMES; ++j)
		{
			a = 2 * b - 1.33;
		}
		
		for(j = 0; j < INNER_LIMES; ++j)
		{
			b = 0.5 * a - 0.45;
		}
		
		for(j = 0; j < INNER_LIMES; ++j)
		{
			a = 2 * b - 1.33;
		}
		
		for(j = 0; j < INNER_LIMES; ++j)
		{
			b = 0.5 * a - 0.45;
		}
	}

	/*printf("%f\n%f\n", a, b);*/

	return 0;
}
