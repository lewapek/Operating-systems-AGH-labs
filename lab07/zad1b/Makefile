CC = gcc
SOURCES = server.c client.c
TARGETS = server client
CFLAGS = -Wall
LFLAGS = -lrt

.PHONY: all
all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)

