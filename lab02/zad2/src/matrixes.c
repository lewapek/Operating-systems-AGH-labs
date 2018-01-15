#include "matrixes.h"
#include <mem_management/mem_management.h>

int matrixes_init(int block_quantity)
{
	return mem_init(block_quantity);
}

void matrixes_finish()
{
	mem_finish();
}

int read_dim(FILE *fp, int *rowA, int *colA, int *rowB, int *colB)
{
	if(fscanf(fp, "%d %d %d %d", rowA, colA, rowB, colB) != 4)
	{
		return 1;
	}

	return 0;
}

type **allocate_matrix(int row, int col)
{
	int i;

	type **result = (type **)mem_allocate(row * sizeof(type *));
	for(i = 0; i < row; ++i)
	{
		result[i] = (type *)mem_allocate(col * sizeof(type));
	}

	return result;
}

void free_matrix(type **matrix, int row)
{
	int i;

	for(i = 0; i < row; ++i)
	{
		mem_deallocate(matrix[i]);
	}

	mem_deallocate(matrix);
}

int read_matrix(FILE *fp, type **matrix, int row, int col)
{
	int i, j;

	for(i = 0; i < row; ++i)
	{
		for(j = 0;j < col; ++j)
		{
			if(!fscanf(fp, IND, &matrix[i][j]))
			{
				return 1;
			}
		}
	}

	return 0;
}

void print_matrix(FILE *fp, type **matrix, int row, int col)
{
	int i, j;

	for(i = 0; i < row; ++i)
	{
		for(j = 0; j < col; ++j)
		{
			fprintf(fp, "\t%.2f", matrix[i][j]);
		}
		fputc('\n', fp);
	}
}

void add_matrix(type **matrixA, type **matrixB, type **result, int row, int col)
{
	int i, j;

	for(i = 0; i < row; ++i)
	{
		for(j = 0; j < col; ++j)
		{
			result[i][j] = matrixA[i][j] + matrixB[i][j];
		}
	}
}

void mul_matrix(type **matrixA, int rowA, int colA, type **matrixB, int rowB, int colB, type **result)
{
	int i, j, k;

	for(i = 0; i < rowA; ++i)
	{
		for(j = 0; j < colB; ++j)
		{
			result[i][j] = 0;

			for(k = 0; k < colA; ++k)
			{
				result[i][j] += matrixA[i][k] * matrixB[k][j];
			}
		}
	}
}

