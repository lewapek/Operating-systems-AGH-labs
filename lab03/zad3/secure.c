#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "secure.h"

static int do_fcntl(int fd, int cmd, off_t offset, int type);
static int get_lock_type(int fd, off_t offset);

int lock_byte_R(int fd, off_t offset)
{
	if(do_fcntl(fd, F_SETLK, offset, F_RDLCK) == -1)
	{
		if(errno == EACCES || errno == EAGAIN)
		{
			return LOCKED_BYTE;
		}

		return -1;
	}

	return 0;
}

int lock_byte_W(int fd, off_t offset)
{
	if(do_fcntl(fd, F_SETLK, offset, F_WRLCK) == -1)
	{
		if(errno == EACCES || errno == EAGAIN)
		{
			return LOCKED_BYTE;
		}

		return -1;
	}

	return 0;
}

int unlock_byte(int fd, off_t offset)
{
	if(do_fcntl(fd, F_SETLK, offset, F_UNLCK)==-1)
	{
		if(errno == EACCES || errno == EAGAIN)
		{
			return LOCKED_BYTE;
		}

		return -1;
	}

	return 0;
}

int read_byte(int fd, off_t offset, char *buffer)
{
	if(get_lock_type(fd, offset) == F_WRLCK)
	{
		return LOCKED_BYTE;
	}
	else if(lseek(fd, offset, SEEK_SET)==-1)
	{
		return -1;
	}
	else if(read(fd, buffer, sizeof(char))==-1)
	{
		return -1;
	}

	return 0;
}

int write_byte(int fd, off_t offset, char *buffer)
{
	if(get_lock_type(fd, offset) != F_UNLCK)
	{
		return LOCKED_BYTE;
	}
	else if(lseek(fd, offset, SEEK_SET)==-1)
	{
		return -1;
	}
	else if(write(fd, buffer, sizeof(char))==-1)
	{
		return -1;
	}
	
	return 0;
}

int print_locks(int fd, FILE *stream)
{
	char *lock_type;
	struct flock lock;
	int locks_counter = 0;
	
	off_t size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	off_t curr;
	for(curr = 0; curr < size; ++curr)
	{
		lock.l_type = F_WRLCK;
		lock.l_whence = SEEK_SET;
		lock.l_len = 1;
		lock.l_start = curr;

		if(fcntl(fd, F_GETLK, &lock) == -1)
		{
			return -1;
		}

		if(lock.l_type != F_UNLCK)
		{
			if(lock.l_type == F_RDLCK)
			{
				lock_type = "read-";
			}
			else if(lock.l_type == F_WRLCK)
			{
				lock_type = "write-";
			}
			else
			{
				lock_type = "";
			}

			fprintf(stream, "PID %d has %slocked byte %d.\n", (int) lock.l_pid, lock_type, (int) lock.l_start);
			++locks_counter;
		}
	}

	if(locks_counter == 0)
	{
		fprintf(stream, "There are no locks for this file from other processes.\n");
	}
	
	return 0;
}

static int do_fcntl(int fd, int cmd, off_t offset, int type)
{
	struct flock lock;
	
	lock.l_type = type;
	lock.l_whence = SEEK_SET;
	lock.l_start = offset;
	lock.l_len = 1;
	
	return fcntl(fd, cmd, &lock);
}

static int get_lock_type(int fd, off_t offset)
{
	struct flock lock;
	
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = offset;
	lock.l_len = 0;

	if(fcntl(fd, F_GETLK, &lock) == -1)
	{
		perror("Function get_lock_type failed");
		fprintf(stderr, "Offset: %d\n", (int)offset);
		
		return -1;
	}

	return lock.l_type;
}
