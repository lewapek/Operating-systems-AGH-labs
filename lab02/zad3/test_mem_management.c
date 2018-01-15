#include <stdio.h>
#include <stdlib.h>
#include <mem_management/mem_management.h>
#include "stat.h"

#ifdef DYNAMIC
#include <dlfcn.h>
#endif

void (*m_print_debug_info)();
int (*m_init)(int);
void *(*m_allocate)(int);
int (*m_deallocate)(void *);
void (*m_finish)(void);

int main()
{
#ifdef DYNAMIC
	void *handle_mem = dlopen("libmem_management.so", RTLD_NOW);
	if(handle_mem == NULL)
	{
		fprintf(stderr, "%s\n", "FATAL ERROR test_mem");
		exit(1);
	}

	m_print_debug_info = dlsym(handle_mem, "mem_print_debug_info");
	m_init = dlsym(handle_mem, "mem_init");
	m_allocate = dlsym(handle_mem, "mem_allocate");
	m_deallocate = dlsym(handle_mem, "mem_deallocate");
	m_finish = dlsym(handle_mem, "mem_finish");
	// dlclose(handle_mem);
#else
	m_print_debug_info = mem_print_debug_info;
	m_init = mem_init;
	m_allocate = mem_allocate;
	m_deallocate = mem_deallocate;
	m_finish = mem_finish;
#endif
	FILE *fp = stdout;

	stat_init();

	(*m_init)(13);

	/*CHECKPOINT*/
	stat_show(fp);

	(*m_print_debug_info)();
	(*m_init)(10000);

	/*CHECKPOINT*/
	stat_show(fp);

	int *array = (*m_allocate)(100 * sizeof(int));
	if(array == NULL)
	{
		fprintf(stderr, "Unable to mem_allocate().\n");
		exit(EXIT_FAILURE);
	}

	int i;
	for(i = 0; i < 100; ++i)
	{
		array[i] = i;
	}

	/*CHECKPOINT*/
	stat_show(fp);

	void * void_ptr = (*m_allocate)(5013);
	if(void_ptr == NULL)
	{
		fprintf(stderr, "Unable to mem_allocate().\n");
		exit(EXIT_FAILURE);
	}

	/*CHECKPOINT*/
	stat_show(fp);

	(*m_deallocate)(array);

	/*CHECKPOINT*/
	stat_show(fp);

	array = (*m_allocate)(13 * sizeof(int));
	if(array == NULL)
	{
		fprintf(stderr, "Unable to mem_allocate().\n");
		exit(EXIT_FAILURE);
	}

	/*CHECKPOINT*/
	stat_show(fp);

	(*m_deallocate)(void_ptr);
	(*m_deallocate)(array);

	/*CHECKPOINT*/
	stat_show(fp);

	(*m_finish)();

	return 0;
}
