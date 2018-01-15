CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
EXECS = parent child

.PHONY: all
all: $(EXECS)

$(EXECS): %:%.c
	  $(CC) -o $@ $(CFLAGS) $<

.PHONY: run
run: all
	./parent 2 child

.PHONY: clean
clean:
	rm -f $(EXECS)

