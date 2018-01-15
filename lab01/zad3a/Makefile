CC=gcc
CFLAGS=-Wall -pedantic -std=c99
SRCS=main.c matrixes.c
OBJS=$(SRCS:.c=.o)
EXEC=main

ifeq ($(DOUBLE), 1)
CFLAGS += -D DOUBLE
endif

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c matrixes.h

matrixes.o: matrixes.c matrixes.h

.PHONY: clean
clean:
	rm $(OBJS)

