CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
EXECS = processes_fork processes_vfork processes_clone_as_fork processes_clone_as_vfork
RM = rm -f
N_VALUES = 20000 40000 80000 160000

all: $(EXECS)

$(EXECS): %:%.o child_parent.o main.o

.PHONY: clean
clean:
	$(RM) $(EXECS) *.o *.points

.PHONY: test
test: $(EXECS)
	./test.sh

