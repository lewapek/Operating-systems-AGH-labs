#include "bst.h"
#include <stdlib.h>

/*local functions*/
static void local_bst_delete_all_nodes(bst_node *node);
static void local_bst_transplant(bst *b, bst_node *x, bst_node *y);

int bst_init(bst *b)
{
	b->root = NULL;
	b->size = 0;

	return 0;
}

void bst_finish(bst *b)
{
	local_bst_delete_all_nodes(b->root);
	b->size = 0;
	b->root = NULL;
}

int bst_add_node(bst *b, bst_data data)
{
	bst_node *node = malloc(sizeof(bst_node));
	if(node == NULL)
	{
		return 1;
	}

	node->data = data;
	node->left = NULL;
	node->right = NULL;

	if(b->root == NULL)
	{
		b->root = node;
		++b->size;

		return 0;
	}

	bst_node *walker = b->root;
	bst_node *pref_walker;
	while(walker != NULL)
	{
		pref_walker = walker;

		if(data.block_quantity < walker->data.block_quantity)
		{
			walker = walker->left;
		}
		else
		{
			walker = walker->right;
		}
	}

	node->parent = pref_walker;

	if(data.block_quantity < pref_walker->data.block_quantity)
	{
		pref_walker->left = node;
	}
	else
	{
		pref_walker->right = node;
	}

	++b->size;

	return 0;
}

void bst_delete_node(bst *b, bst_node *node)
{
	if(node->left == NULL)
	{
		local_bst_transplant(b, node, node->right);
	}
	else if(node->right == NULL)
	{
		local_bst_transplant(b, node, node->left);
	}
	else
	{
		bst_node *minimum = node->right;
		while(minimum->left != NULL)
		{
			minimum = minimum->left;
		}

		if(minimum->parent != node)
		{
			local_bst_transplant(b, minimum, minimum->right);
			minimum->right = node->right;
			minimum->right->parent = minimum;
		}

		local_bst_transplant(b, node, minimum);
		minimum->left = node->left;
		minimum->left->parent = minimum;
	}

	free(node);
	--b->size;
}

bst_node *bst_find_min_node(bst *b, int min_block_quantity)
{
	if(b->root == NULL)
	{
		return NULL;
	}

	bst_node *result = NULL;
	bst_node *walker = b->root;

	while(walker != NULL && walker->data.block_quantity >= min_block_quantity)
	{
		result = walker;
		walker = walker->left;
	}

	return result;
}

bst_node *bst_find_max_node(bst *b, int min_block_quantity)
{
	if(b->root == NULL)
	{
		return NULL;
	}

	bst_node *pref_walker;
	bst_node *walker = b->root;

	while(walker != NULL)
	{
		pref_walker = walker;
		walker = walker->right;
	}

	return (pref_walker->data.block_quantity >= min_block_quantity) ? pref_walker : NULL;
}

/*local functions*/
static void local_bst_delete_all_nodes(bst_node *node)
{
	if(node == NULL)
	{
		return;
	}

	local_bst_delete_all_nodes(node->left);
	local_bst_delete_all_nodes(node->right);
	free(node);
}

static void local_bst_transplant(bst *b, bst_node *x, bst_node *y)
{
	if(x->parent == NULL)
	{
		b->root = y;
	}
	else if(x == x->parent->left)
	{
		x->parent->left = y;
	}
	else
	{
		x->parent->right = y;
	}

	if(y != NULL)
	{
		y->parent = x->parent;
	}
}
