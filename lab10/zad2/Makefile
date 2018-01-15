CC = gcc
CFLAGS = -Wall -g -I. -pthread
LFLAGS = -pthread
OBJS = server.o client.o

all : $(OBJS)

%.o : %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) -o $* $@ $(LFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(OBJS:.o=)

