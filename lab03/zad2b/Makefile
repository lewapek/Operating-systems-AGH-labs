CC = gcc
CFLAGS = -Wall -pedantic -std=c99
SRCS = main.c dir_search.c
OBJS = $(SRCS:.c=.o)

all: program

program: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

main.o: main.c dir_search.o

dir_search.o: dir_search.c dir_search.h

.PHONY: clean
clean:
	rm -f program *.o

