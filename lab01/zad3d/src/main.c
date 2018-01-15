#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrixes.h"

#define AVAILABLE_OPERATIONS "*+"
#define USAGE fprintf(stderr, "\tUsage: %s file operation\n\tOperation is one of: {*, +}.\n", argv[0])

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		fprintf(stderr, "Bad number of arguments.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	if(strlen(argv[2]) != 1 || strstr(AVAILABLE_OPERATIONS, argv[2]) == NULL)
	{
		fprintf(stderr, "Bad operator given.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		fprintf(stderr, "Unable to open file.\n");
		USAGE;
		exit(EXIT_FAILURE);
	}

	int rowA, colA, rowB, colB;
	read_dim(fp, &rowA, &colA, &rowB, &colB);

	type **matrixA = allocate_matrix(rowA, colA);
	type **matrixB = allocate_matrix(rowB, colB);

	if(read_matrix(fp, matrixA, rowA, colA) != 0)
	{
		fprintf(stderr, "Error reading matrixes.\n");
		exit(EXIT_FAILURE);
	}

	if(read_matrix(fp, matrixB, rowB, colB) != 0)
	{
		fprintf(stderr, "Error reading matrixes.\n");
		exit(EXIT_FAILURE);
	}

	printf("A:\n");
	print_matrix(matrixA, rowA, colA);
	printf("\nB:\n");
	print_matrix(matrixB, rowB, colB);

	if(argv[2][0] == '+')
	{
		if(rowA != rowB || colA != colB)
		{
			fprintf(stderr, "Bad matrix dimensions. Unable to add.\n");
			exit(EXIT_FAILURE);
		}

		type **result = allocate_matrix(rowA, colA);
		add_matrix(matrixA, matrixB, result, rowA, colA);

		printf("\nA + B:\n");
		print_matrix(result, rowA, colA);

		free_matrix(result, rowA);
	}
	else
	{
		if(colA != rowB)
		{
			fprintf(stderr, "Bad matrix dimensions. Unable to multiplicate.\n");
			exit(EXIT_FAILURE);
		}

		type **result = allocate_matrix(rowA, colB);
		mul_matrix(matrixA, rowA, colA, matrixB, rowB, colB, result);

		printf("\nA * B:\n");
		print_matrix(result, rowA, colB);

		free_matrix(result, rowA);
	}

	free_matrix(matrixB, rowB);
	free_matrix(matrixA, rowA);

	fclose(fp);

	return 0;
}
