CC = gcc
SRCS = main.c
TARGET = main
CFLAGS = -std=c99 -Wall

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS)

.PHONY: clean
clean:
	rm -f main message.gz

