CC = gcc
LFLAGS = -lpthread -std=c99 -Wall
SRC = finder.c

.PHONY: all
all: finder1 finder2 finder3

finder1: finder.c
	$(CC) $(SRC) -o $@ $(CFLAGS) $(LFLAGS)

finder2: finder.c
	$(CC) $(SRC) -o $@ -DSYNCHR $(CFLAGS) $(LFLAGS)

finder3: finder.c
	$(CC) $(SRC) -o $@ -DDETACH $(CFLAGS) $(LFLAGS)

.PHONY: clean
clean:
	rm -f finder?

