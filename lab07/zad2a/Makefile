CC = gcc
SOURCES = server.c client.c
TARGETS = server client
CFLAGS = -Wall

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES) header.h
	$(CC) -o $@ $@.c $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS) messages.txt

