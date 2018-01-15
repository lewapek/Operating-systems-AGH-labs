CC = gcc
SOURCES = server.c client.c
TARGETS = server client
CFLAGS = -std=c99 -Wall

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CC) -o $@ $@.c $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)

