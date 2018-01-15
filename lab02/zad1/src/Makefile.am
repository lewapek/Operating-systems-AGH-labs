INCLUDE_DIR_PATH_BEGIN=/usr/local/include

lib_LTLIBRARIES=libmem_management.la
libmem_management_la_SOURCES=mem_management.c mem_management.h bst.c bst.h list.c list.h
libmem_management_la_includedir=$(INCLUDE_DIR_PATH_BEGIN)/mem_management
libmem_management_la_include_HEADERS=mem_management.h bst.h list.h
libmem_management_la_CFLAGS=-Wall -std=c99 -g -pg -I.
