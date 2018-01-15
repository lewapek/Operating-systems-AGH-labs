CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
EXECS = sender receiver

all: $(EXECS)

$(EXECS): %:%.c
	  $(CC) -o $@ $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(EXECS)

