#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

#ifndef _SECURE_H_
#define _SECURE_H_

#define LOCKED_BYTE -0x20

int lock_byte_R(int fd, off_t offset);
int lock_byte_W(int fd, off_t offset);
int unlock_byte(int fd, off_t offset);
int read_byte(int fd, off_t offset, char *buffer);
int write_byte(int fd, off_t offset, char *buffer);
int print_locks(int fd, FILE *stream);

#endif /*_SECURE_H_*/