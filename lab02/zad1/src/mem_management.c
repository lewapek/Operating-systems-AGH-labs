#include "mem_management.h"
#include "bst.h"
#include "list.h"
#include <stdlib.h>
#include <stdio.h>

extern mem_management m;

/*local functions*/
static void *local_mem_allocate(int block_quantity);
static void local_mem_defragmentation();

int mem_init(int block_quantity)
{
	int check;

	m.buffer_address = malloc(block_quantity * BLOCK_SIZE);	
	if(m.buffer_address == NULL)
	{
		return 1;
	}

	m.block_quantity = block_quantity;
	
	check = bst_init(&m.empty_blocks_bst);
	if(check != 0)
	{
		return 2;
	}

	bst_data start_data;
	start_data.address = m.buffer_address;
	start_data.block_quantity = block_quantity;
	bst_add_node(&m.empty_blocks_bst, start_data);

	check = list_init(&m.reserved_blocks_list);
	if(check != 0)
	{
		return 3;
	}

	return 0;
}

void mem_finish()
{
	free(m.buffer_address);
	bst_finish(&m.empty_blocks_bst);
	list_finish(&m.reserved_blocks_list);
}

void *mem_allocate(int size)
{
	if(size <= 0 || size > m.block_quantity * BLOCK_SIZE)
	{
		return NULL;
	}

	int block_quantity = size / BLOCK_SIZE;
	if((double)size / BLOCK_SIZE > block_quantity)
	{
		++block_quantity;
	}

	//try to allocate
	void *result = local_mem_allocate(block_quantity);
	if(result != NULL)
	{
		return result;
	}	

	//here there is not enough space, merge needed
	local_mem_defragmentation();

	//after defragmentation
	result = local_mem_allocate(block_quantity);

	return result;
}

int mem_deallocate(void *address)
{
	list_node *walker = m.reserved_blocks_list.head->next;
	while(walker != NULL && walker->data.address != address)
	{
		walker = walker->next;
	}

	if(walker == NULL)
	{
		return 1;
	}

	bst_data new_data;
	new_data.address = walker->data.address;
	new_data.block_quantity = walker->data.block_quantity;
	bst_add_node(&m.empty_blocks_bst, new_data);

	list_delete_node(&m.reserved_blocks_list, walker);

	return 0;
}

mem_diagnostic_info mem_diagnose()
{
	mem_diagnostic_info result;
	result.empty_elements_quantity = m.empty_blocks_bst.size;
	result.reserved_elements_quantity = m.reserved_blocks_list.size;

	if(m.empty_blocks_bst.root == NULL)
	{
		result.min_empty_element_size = 0;
		result.max_empty_element_size = 0;

		return result;
	}

	bst_node *node = m.empty_blocks_bst.root;
	while(node->left != NULL)
	{
		node = node->left;
	}
	result.min_empty_element_size = node->data.block_quantity * BLOCK_SIZE;

	node = m.empty_blocks_bst.root;
	while(node->right != NULL)
	{
		node = node->right;
	}
	result.max_empty_element_size = node->data.block_quantity * BLOCK_SIZE;

	return result;
}

void mem_print_diagnostic_info(FILE *fp, mem_diagnostic_info info)
{
	fprintf(fp, "Diagnostic information:\n");
	fprintf(fp, "\tEmpty elements quantity: %d\n", info.empty_elements_quantity);
	fprintf(fp, "\tReserved elements quantity: %d\n", info.reserved_elements_quantity);
	fprintf(fp, "\tMinimum empty element size: %d\n", info.min_empty_element_size);
	fprintf(fp, "\tMaximum empty element size: %d\n", info.max_empty_element_size);
}

/*local functions*/
static void *local_mem_allocate(int block_quantity)
{
	//choosing between strategies
#ifdef MIN_ALLOC_STRATEGY
	bst_node *empty_node = bst_find_min_node(&m.empty_blocks_bst, block_quantity);
#else /*MAX_ALLOC_STRATEGY*/
	bst_node *empty_node = bst_find_max_node(&m.empty_blocks_bst, block_quantity);
#endif

	//if there is enough space in empty node
	if(empty_node == NULL)
	{
		return NULL;
	}
	
	//adding new node to list
	list_data data;
	data.block_quantity = block_quantity;
	data.address = empty_node->data.address;
	list_add_node(&m.reserved_blocks_list, data);

	int block_quantity_diff = empty_node->data.block_quantity - block_quantity;
	
	//removing node from bst
	bst_delete_node(&m.empty_blocks_bst, empty_node);

	//optional insert node into bst, in case when there is still empty space after allocation (empty space connected with the empty_node)
	if(block_quantity_diff > 0)
	{
		bst_data new_data;
		new_data.block_quantity = block_quantity_diff;
		new_data.address = data.address + data.block_quantity * BLOCK_SIZE;
		bst_add_node(&m.empty_blocks_bst, new_data);
	}

	return data.address;
}

static void local_mem_defragmentation()
{
	//creating new empty_blocks_bst
	void *begin = m.buffer_address;
	list_node *list_walker = m.reserved_blocks_list.head->next;
	void *end = list_walker->data.address;
	
	while(begin == end)
	{
		printf("%s\n", "a");
		begin = end + list_walker->data.block_quantity * BLOCK_SIZE;
		list_walker = list_walker->next;
		if(list_walker == NULL)
		{
			return;
		}

		end = list_walker->data.address;
	}

	bst_finish(&m.empty_blocks_bst);
	bst_init(&m.empty_blocks_bst);

	while(list_walker != NULL)
	{
		printf("%s, %p, %p\n", "INSIDE WHILE", begin, end);
		if(end > begin)
		{
			bst_data new_data;
			new_data.block_quantity = (end - begin) / BLOCK_SIZE;
			new_data.address = begin;

			bst_add_node(&m.empty_blocks_bst, new_data);

			begin = end + list_walker->data.block_quantity;
			list_walker = list_walker->next;

			while(begin == end)
			{
				begin = end + list_walker->data.block_quantity * BLOCK_SIZE;
				list_walker = list_walker->next;
				if(list_walker == NULL)
				{
					break;
				}
				
				end = list_walker->data.address;
			}
		}
		else
		{
			fprintf(stderr, "Memory defragmenting error occured.\n");
			return;//some error occured
		}
	}

	//checking last fragment
	if(begin < m.buffer_address + m.block_quantity * BLOCK_SIZE)
	{
		bst_data new_data;
		new_data.address = begin;
		new_data.block_quantity = (m.buffer_address + m.block_quantity * BLOCK_SIZE - begin) / BLOCK_SIZE;

		bst_add_node(&m.empty_blocks_bst, new_data);
	}
}

void mem_print_debug_info()
{
	printf("BLOCK_SIZE = %d\n", BLOCK_SIZE);
#ifdef MIN_ALLOC_STRATEGY
	printf("Alloc strategy = MIN_ALLOC_STRATEGY\n");
#endif
#ifdef MAX_ALLOC_STRATEGY
	printf("Alloc strategy = MAX_ALLOC_STRATEGY\n");
#endif
}
