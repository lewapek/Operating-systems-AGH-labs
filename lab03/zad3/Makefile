CC = gcc
CFLAGS = -Wall -pedantic -std=c99
SRCS = main.c secure.c
OBJS = $(SRCS:.c=.o)

all: program

program: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $@

main.o: main.c secure.o

secure.o: secure.c secure.h

.PHONY: clean
clean:
	rm -f program *.o

