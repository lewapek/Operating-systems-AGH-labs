CC = gcc
CFLAGS = -Wall -pedantic
SRCS = signal.c sigaction.c
EXECS = $(SRCS:.c=)

all: $(EXECS)

signal: signal.c

sigaction: sigaction.c 

.PHONY: clean
clean:
	rm -f $(EXECS)

