#ifndef _LIST_H_
#define _LIST_H_

typedef struct list_data_struct
{
	void *address;
	int block_quantity;
} list_data;

typedef struct list_node_struct
{
	list_data data;
	struct list_node_struct *next;
} list_node;

typedef struct list_struct
{
	list_node *head;
	int size;
} list;

int list_init(list *li);
void list_finish(list *li);
int list_add_node(list *li, list_data data);
void list_delete_node(list *li, list_node *node);

#endif /*_LIST_H_*/
