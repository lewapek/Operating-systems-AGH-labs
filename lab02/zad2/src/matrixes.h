#ifndef _MATRIXES_H_
#define _MATRIXES_H_

#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define AVAILABLE_OPERATIONS "*+"

#ifdef DOUBLE
	typedef double type;
	#define IND "%lf"
#else
	typedef float type;
	#define IND "%f"
#endif

int matrixes_init(int block_quantity);
void matrixes_finish();

int read_dim(FILE *fp, int *rowA, int *colA, int *rowB, int *colB);
type **allocate_matrix(int row, int col);
void free_matrix(type **matrix, int row);
int read_matrix(FILE *fp, type **matrix, int row, int col);
void print_matrix(FILE *fp, type **matrix, int row, int col);
void add_matrix(type **matrixA, type **matrixB, type **result, int row, int col);
void mul_matrix(type **matrixA, int rowA, int colA, type **matrixB, int rowB, int colB, type **result);

#endif /*_MATRIXES_H_*/
