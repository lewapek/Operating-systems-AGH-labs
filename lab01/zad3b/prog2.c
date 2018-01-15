#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SIZE 50000

void swap(double *a, double *b)
{
	double c = *a;
	*a = *b;
	*b = c;
}

void bubble_sort(double *array, int size)
{
	int i, j;

	for(i = 0; i < size; ++i)
	{
		for(j = size - 1; j > i; --j)
		{
 			if(array[j - 1] > array[j])
 			{
				swap(&array[j - 1], &array[j]);
			}
		}
	}
}

void print_array(double *array, int size)
{
	int i;

	for(i = 0; i < size; ++i)
	{
		printf("  %.2f", array[i]);
	}

	putchar('\n');
}

int main()
{
	double *array = malloc(SIZE * sizeof(double));
	int i;
	
	srand(time(NULL));
	
	for(i = 0; i < SIZE; ++i)
	{
		array[i] = (rand() / (double)RAND_MAX) * (rand() % 101);
	}

/*	print_array(array, SIZE);
*/
	bubble_sort(array, SIZE);
/*	print_array(array, SIZE);
*/
	return 0;
}
