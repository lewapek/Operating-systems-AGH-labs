CC = gcc
SOURCES = server.c client.c
TARGETS = server client
CFLAGS = -Wall

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES) config_header.h
	$(CC) -o $@ $@.c $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)

