CC = gcc
SOURCES = producer.c consumer.c runner.c
TARGETS = producer consumer runner
CFLAGS = -Wall -pedantic -std=c99
LFLAGS = 

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES) header.h
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)

