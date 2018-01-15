CC = gcc
CFLAGS = -Wall -pedantic -std=c99

all : system library
s%:CFLAGS+=-DSYS
system: sprog
library: lprog

l%:%.c
	$(CC) $^ -o $@ $(CFLAGS) 
	
s%:%.c
	$(CC) $^ -o $@ $(CFLAGS) 

.PHONY: clean
clean:
	rm -f sprog lprog

.PHONY: test
test:
	./test.sh
