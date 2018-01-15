#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <matrixes/matrixes.h>
#include "stat.h"

#ifdef DYNAMIC
#include <dlfcn.h>
#endif

int (*mat_init)(int);
int (*mat_read_dim)(FILE *, int *, int *, int *, int *);
type** (*mat_allocate_matrix)(int, int);
int (*mat_read_matrix)(FILE *, type**, int, int);
void (*mat_print_matrix)(FILE *, type**, int, int);
void (*mat_free_matrix)(type **, int);
void (*mat_add_matrix)(type **, type **, type **, int, int);
void (*mat_mul_matrix)(type **, int, int, type **, int, int, type **);
void (*mat_finish)();

#define USAGE fprintf(stderr, "\tUsage: %s file operation\n\tOperation is one of: {*, +}.\n", argv[0])

int main(int argc, char **argv)
{
#ifdef DYNAMIC
	void *handle_mat = dlopen("libmatrixes.so", RTLD_NOW);
	if(handle_mem == NULL)
	{
		fprintf(stderr, "%s\n", "FATAL ERROR test_matrixes");
		exit(1);
	}

	mat_init = dlsym(handle_mat, "matrixes_init");
	mat_read_dim = dlsym(handle_mat, "read_dim");
	mat_allocate_matrix = dlsym(handle_mat, "allocate_matrix");
	mat_read_matrix = dlsym(handle_mat, "read_matrix");
	mat_print_matrix = dlsym(handle_mat, "print_matrix");
	mat_free_matrix = dlsym(handle_mat, "free_matrix");
	mat_add_matrix = dlsym(handle_mat, "add_matrix");
	mat_mul_matrix = dlsym(handle_mat, "mul_matrix");
	mat_finish = dlsym(handle_mat, "matrixes_finish");
	// dlclose(handle_mem);
#else
	mat_init = matrixes_init;
	mat_read_dim = read_dim;
	mat_allocate_matrix = allocate_matrix;
	mat_read_matrix = read_matrix;
	mat_print_matrix = print_matrix;
	mat_free_matrix = free_matrix;
	mat_add_matrix = add_matrix;
	mat_mul_matrix = mul_matrix;
	mat_finish = matrixes_finish;
#endif
	
	FILE *stream = stdout;

	stat_init();
	/*CHECKPOINT*/
	stat_show(stream);

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

	(*mat_init)(1000);

	int rowA, colA, rowB, colB;
	if((*mat_read_dim)(fp, &rowA, &colA, &rowB, &colB) != 0)
	{
		fprintf(stderr, "Error reading matrixes dimensions.\n");
		exit(EXIT_FAILURE);
	}

	/*CHECKPOINT*/
	stat_show(stream);

	type **matrixA = (*mat_allocate_matrix)(rowA, colA);
	
	/*CHECKPOINT*/
	stat_show(stream);
	
	type **matrixB = (*mat_allocate_matrix)(rowB, colB);

	if((*mat_read_matrix)(fp, matrixA, rowA, colA) != 0)
	{
		fprintf(stderr, "Error reading matrixes.\n");
		exit(EXIT_FAILURE);
	}

	if((*mat_read_matrix)(fp, matrixB, rowB, colB) != 0)
	{
		fprintf(stderr, "Error reading matrixes.\n");
		exit(EXIT_FAILURE);
	}

	/*CHECKPOINT*/
	stat_show(stream);

	printf("A:\n");
	(*mat_print_matrix)(fp, matrixA, rowA, colA);
	printf("\nB:\n");
	(*mat_print_matrix)(fp, matrixB, rowB, colB);

	/*CHECKPOINT*/
	stat_show(stream);

	if(argv[2][0] == '+')
	{
		if(rowA != rowB || colA != colB)
		{
			fprintf(stderr, "Bad matrix dimensions. Unable to add.\n");
			exit(EXIT_FAILURE);
		}

		type **result = (*mat_allocate_matrix)(rowA, colA);
		(*mat_add_matrix)(matrixA, matrixB, result, rowA, colA);

		printf("\nA + B:\n");
		(*mat_print_matrix)(fp, result, rowA, colA);

		(*mat_free_matrix)(result, rowA);
	}
	else
	{
		if(colA != rowB)
		{
			fprintf(stderr, "Bad matrix dimensions. Unable to multiplicate.\n");
			exit(EXIT_FAILURE);
		}

		type **result = (*mat_allocate_matrix)(rowA, colB);
		(*mat_mul_matrix)(matrixA, rowA, colA, matrixB, rowB, colB, result);

		printf("\nA * B:\n");
		(*mat_print_matrix)(fp, result, rowA, colB);

		(*mat_free_matrix)(result, rowA);
	}

	/*CHECKPOINT*/
	stat_show(stream);

	(*mat_free_matrix)(matrixB, rowB);
	/*CHECKPOINT*/
	stat_show(stream);
	(*mat_free_matrix)(matrixA, rowA);

	fclose(stream);

	/*CHECKPOINT*/
	stat_show(stream);
	(*mat_finish)();

	return 0;
}
