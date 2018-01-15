#ifndef _MEM_MANAGEMENT_H_
#define _MEM_MANAGEMENT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "list.h"
#include "bst.h"
#include <stdio.h>

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 8
#endif /*BLOCK_SIZE*/

#ifndef ALLOC_STRATEGY
#define ALLOC_STRATEGY
#define MIN_ALLOC_STRATEGY
#endif

typedef struct mem_management_struct
{
	void *buffer_address;
	int block_quantity;
	bst empty_blocks_bst;
	list reserved_blocks_list;
} mem_management;

typedef struct mem_diagnostic_info_struct
{
	int empty_elements_quantity;
	int reserved_elements_quantity;
	int min_empty_element_size;
	int max_empty_element_size;
} mem_diagnostic_info;

//global variable
mem_management m;

int mem_init(int block_quantity);
void mem_finish();
void *mem_allocate(int size);
int mem_deallocate(void *address);
mem_diagnostic_info mem_diagnose();
void mem_print_diagnostic_info(FILE *fp, mem_diagnostic_info info);
void mem_print_debug_info();

#endif /*_MEM_MANAGEMENT_H_*/
