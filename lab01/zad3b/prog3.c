#include <stdio.h>
#include <math.h>
#define LIMES 2e7

int main()
{
	long i;
	double a = 1.0;

	for (i = 0; i < LIMES; ++i)
	{
		double b = cos(tan(a + 0.0001 * 23.32 / 0.54));
		a = sqrt(log(fabs(a + i)) + pow(2, 2.1));
		a = sin(a + 0.5423 * exp2(0.32 + i / (i + 1))) + log(fabs(b));
		b = fabs(b);
		b = log(log(log(log(log(log(log(log(log(log(log(b)))))))))));
		b = fabs(b);
		b = sqrt(sqrt(sqrt(sqrt(sqrt(sqrt(sqrt(sqrt(sqrt(sqrt(exp(b)))))))))));
		a += b;
	}

	/*printf("%f\n", a);*/

	return 0;
}
