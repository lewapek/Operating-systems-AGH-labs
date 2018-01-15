#include <stdio.h>
#define LIMES 5e8

int main()
{
	long i = 0;
	double a = 1;

	for (; i < LIMES; ++i)
	{
		if(i % 2 == 0)
		{
			a *= (i / (i + 1) + 0.54 * 0.123 * 1.12 * 1.13) / a * (a + 0.0001);
		}
		else
		{
			a *= (i / (i + 1) + 0.543 * 0.123 * 2.2) + 0.11 * 0.43 * a;
		}
	}

	/*printf("%f\n", a);*/

	return 0;
}

