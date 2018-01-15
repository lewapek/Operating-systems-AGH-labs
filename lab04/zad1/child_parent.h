#ifndef _REALFUNCTIONS_H
#define _REALFUNCTIONS_H

#include <sys/types.h>
#include <sys/wait.h>

int counter;

int child(void *arg);
int parent();

#endif /*_REALFUNCTIONS_H*/