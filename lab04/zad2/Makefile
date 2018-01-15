CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
EXECS = archivize append

all: $(EXECS) dir_create

$(EXECS):
	$(CC) $@.c -o $@ $(CFLAGS)

dir_create:
	mkdir archive

.PHONY: clean
clean:
	rm -r $(EXECS) archive
