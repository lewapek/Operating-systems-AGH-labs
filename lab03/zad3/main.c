#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "secure.h"

#define USAGE USAGE_PARAM(stderr, argv[0])
#define USAGE_PARAM(fp, name) fprintf(fp, "Usage:\n\t%s filename\n", name)

#define FFLUSH while(getchar() != '\n') continue;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		USAGE;
		return -1;
	}

	char *filename = argv[1];

	int fd = open(filename, O_RDWR);

	if(fd < 0)
	{
		perror("Cannot open file");
		fprintf(stderr, "File name: %s\n", filename);
		return -1;
	}

	int condition = 0;
	char buffer = 0;
	char command[300];
	int offset;

	printf("Locker for file: %s\n", filename);
	printf("Type help for list of commands.\n\n");

	while(condition == 0)
	{
		printf(" >> ");
		scanf("%s", command);
		FFLUSH;

		if(strcmp(command, "help") == 0)
		{
			printf("\n      Available commands\n");
			printf("-----------------------------------\n");
			printf("help       displays this help\n");
			printf("lockread   locks byte for reading (when possible)\n");
			printf("lockwrite  locks byte for writing (when possible)\n");
			printf("unlock     unlocks byte (when possible)\n");
			printf("read       reads byte (when possible)\n");
			printf("write      writes byte (when possible)\n");
			printf("locks      displays locks on file from other processes\n");
			printf("exit       quits locker\n\n");
		}
		else if(strcmp(command, "locks") == 0)
		{
			if(print_locks(fd, stdout) != 0)
			{
				perror("Unable to read locks");
				fprintf(stderr, "File: %s\n", filename);
				condition = 1;
			}
		}
		else if(strcmp(command, "lockread") == 0)
		{
			printf("Type offset to put read lock at: ");
			scanf("%d", &offset);
			FFLUSH;			

			if(offset < 0)
			{
				printf("Invalid offset.\n");
				continue;
			}

			int result = lock_byte_R(fd, (off_t) offset);
			if(result == LOCKED_BYTE)
			{
				printf("Unable to lock on byte %d: byte is already locked at by another process.\n", offset);
				printf("Type locks for more information.\n");
				continue;
			}
			else if(result != 0)
			{
				perror("Unable to lock file for reading");
				fprintf(stderr, "File: %s\nByte: %d\n", filename, offset);
				condition = 1;
			}
			else
			{
				printf("Successfully locked byte %d for reading\n", offset);
			}
		}
		else if(strcmp(command, "lockwrite") == 0)
		{
			printf("Type offset to put write lock at: ");
			scanf("%d", &offset);
			FFLUSH;

			if(offset < 0)
			{
				printf("Invalid.\n");
				continue;
			}

			int result = lock_byte_W(fd, (off_t)offset);
			if(result == LOCKED_BYTE)
			{
				printf("Unable to lock on byte %d: byte is already locked at by another process.\n", offset);
				printf("Type locks for more information.\n");
				continue;
			}
			else if(result != 0)
			{
				perror("Unable to lock file for writing");
				fprintf(stderr, "File: %s\nByte: %d\n", filename, offset);
				condition = 1;
			}
			else
			{
				printf("Successfully locked byte %d for writing\n", offset);
			}
		}
		else if(strcmp(command, "unlock") == 0)
		{
			printf("Type offset to unlock at: ");
			scanf("%d", &offset);
			FFLUSH;

			if(offset < 0)
			{
				printf("Invalid offset.\n");
				continue;
			}

			int result = unlock_byte(fd, (off_t) offset);
			if(result == LOCKED_BYTE)
			{
				printf("Unable to unlock on byte %d: byte is locked by another process.\n", offset);
				printf("Type locks for more information.\n");
				continue;
			}
			else if(result != 0)
			{
				perror("Unable to unlock file for reading");
				fprintf(stderr, "File: %s\nByte: %d\n", filename, offset);
				condition = 1;
			}
			else
			{
				printf("Successfully unlocked byte %d\n", offset);
			}
		}
		else if(strcmp(command, "read") == 0)
		{
			printf("Please type offset to read from:");
			scanf("%d", &offset);
			FFLUSH;

			if(offset < 0)
			{
				printf("Invalid offset.\n");
				continue;
			}

			int result = read_byte(fd, (off_t)offset, &buffer);
			if(result == LOCKED_BYTE)
			{
				printf("Unable to read byte %d: byte is locked by another process.\n", offset);
				printf("Type locks for more information.\n");
				continue;
			}
			else if(result != 0)
			{
				perror("Unable to read");
				fprintf(stderr, "File: %s\nByte: %d\n", filename, offset);
				condition = 1;
			}
			else
			{
				printf("In file %s at offset %d is: %c\n", filename, offset, buffer);
			}
		}
		else if(strcmp(command, "write") == 0)
		{
			printf("Type offset and byte to write to.\n");
			printf("Offset: ");
			scanf("%d", &offset);
			FFLUSH;
			printf("Byte: ");
			scanf("%c", &buffer);
			FFLUSH;

			if(offset < 0)
			{
				printf("Invalid offset.\n");
				continue;
			}

			int result = write_byte(fd, (off_t)offset, &buffer);
			if(result == LOCKED_BYTE)
			{
				printf("Unable to write byte %d: byte is locked by another process.\n", offset);
				printf("Type locks for more information.\n");
				continue;
			}
			else if(result != 0)
			{
				perror("Unable to write");
				fprintf(stderr, "File: %s\nByte: %d\n", filename, offset);
				condition = 1;
			}
			else
			{
				printf("Successfully written %c to file %s at offset %d\n", buffer, filename, offset);
			}
		}
		else if(strcmp(command, "exit") == 0)
		{
			condition = 1;
		}
		else
		{
			printf("Invalid command. Type help for help.\n");
		}
	}

	close(fd);

	return 0;
}

