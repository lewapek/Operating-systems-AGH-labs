CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
EXECS = count

all: $(EXECS)

$(EXECS):
	$(CC) $@.c -o $@ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(EXECS)

