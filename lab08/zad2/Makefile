CC = gcc
SOURCES = reader.c writer.c runner.c
TARGETS = reader writer runner
CFLAGS = -Wall -pedantic -std=c99
LFLAGS = -lrt

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES) header.h
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)

