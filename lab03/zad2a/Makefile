CC = gcc
CFLAGS = -Wall -pedantic -std=c99
SRCS = main.c
OBJS = $(SRCS:.c=.o)
EXEC = main

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJS): $(SRCS)

.PHONY: clean
clean:
	rm $(OBJS) $(EXEC)

.PHONY: run
run: $(EXEC)
	./$(EXEC) . 664

