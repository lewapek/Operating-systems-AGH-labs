#ifndef _BST_H_
#define _BST_H_

typedef struct bst_data_struct
{
	void *address;
	int block_quantity;
} bst_data;

typedef struct bst_node_struct
{
	bst_data data;
	struct bst_node_struct *left, *right, *parent;
} bst_node;

typedef struct bst_struct
{
	bst_node *root;
	int size;
} bst;

int bst_init(bst *b);
void bst_finish(bst *b);
int bst_add_node(bst *b, bst_data data);
void bst_delete_node(bst *b, bst_node *node);
bst_node *bst_find_min_node(bst *b, int min_block_quantity);
bst_node *bst_find_max_node(bst *b, int min_block_quantity);

#endif /*_BST_H_*/
