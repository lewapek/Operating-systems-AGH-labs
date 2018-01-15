#include "list.h"
#include <stdlib.h>

int list_init(list *li)
{
	li->head = malloc(sizeof(list_node));
	if(li->head == NULL)
	{
		return 1;
	}

	list_data data;
	data.address = NULL;
	data.block_quantity = 0;

	li->head->data = data;
	li->head->next = NULL;
	li->size = 0;

	return 0;
}

void list_finish(list *li)
{
	list_node *ptr = li->head;

	if(ptr->next != NULL)
	{
		ptr = ptr->next;

		while(ptr->next != NULL)
		{
			list_node *pref = ptr;
			free(pref);
			ptr = ptr->next;
		}
	}

	free(li->head);
	li->size = 0;
}

int list_add_node(list *li, list_data data)
{
	list_node *node = malloc(sizeof(list_node));
	if(node == NULL)
	{
		return 1;
	}

	node->data = data;

	list_node *pref_walker = li->head;
	list_node *walker = pref_walker->next;
	while(walker != NULL && data.address > walker->data.address)
	{
		pref_walker = walker;
		walker = walker->next;
	}

	pref_walker->next = node;
	node->next = walker;

	++li->size;

	return 0;
}

void list_delete_node(list *li, list_node *node)
{
	list_node *pref_walker = li->head;
	list_node *walker = pref_walker->next;
	while(walker != node)
	{
		pref_walker = walker;
		walker = walker->next;
	}

	pref_walker->next = node->next;
	free(node);
	--li->size;
}
