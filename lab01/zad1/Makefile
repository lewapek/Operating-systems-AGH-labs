CC=gcc
CFLAGS=-lm -Wall -pedantic
LFLAGS=
ARCH=-march=native
CF_PROG1=-O1
CF_PROG2=-O2
CF_PROG3=-O1 -ffast-math
CF_PROG4=-O1 -funroll-loops
P1=prog1.c
P2=prog2.c
P3=prog3.c
P4=prog4.c
PREF1=$(P1:.c=)
PREF2=$(P2:.c=)
PREF3=$(P3:.c=)
PREF4=$(P4:.c=)
SRCS=$(P1) $(P2) $(P3) $(P4)
PROG1_EXECS=$(P1:.c=_norm) $(P1:.c=_norm_arch) $(P1:.c=_opt) $(P1:.c=_opt_arch)
PROG2_EXECS=$(P2:.c=_norm) $(P2:.c=_norm_arch) $(P2:.c=_opt) $(P2:.c=_opt_arch)
PROG3_EXECS=$(P3:.c=_norm) $(P3:.c=_norm_arch) $(P3:.c=_opt) $(P3:.c=_opt_arch)
PROG4_EXECS=$(P4:.c=_norm) $(P4:.c=_norm_arch) $(P4:.c=_opt) $(P4:.c=_opt_arch)
EXECS=$(PROG1_EXECS) $(PROG2_EXECS) $(PROG3_EXECS) $(PROG4_EXECS)
TEST=time -f "user time: %U sec"

.PHONY: all
all: $(EXECS)

$(PROG1_EXECS): $(P1)
	@echo "Program 1"
	$(CC) $(P1) $(CFLAGS) $(LIBS) -o $(PREF1)_norm
	$(CC) $(P1) $(CFLAGS) $(LIBS) $(ARCH) -o $(PREF1)_norm_arch
	$(CC) $(P1) $(CFLAGS) $(CF_PROG1) $(LIBS) -o $(PREF1)_opt
	$(CC) $(P1) $(CFLAGS) $(CF_PROG1) $(LIBS) $(ARCH) -o $(PREF1)_opt_arch

$(PROG2_EXECS): $(P2)
	@echo "Program 2"
	$(CC) $(P2) $(CFLAGS) $(LIBS) -o $(PREF2)_norm
	$(CC) $(P2) $(CFLAGS) $(LIBS) $(ARCH) -o $(PREF2)_norm_arch
	$(CC) $(P2) $(CFLAGS) $(CF_PROG2) $(LIBS) -o $(PREF2)_opt
	$(CC) $(P2) $(CFLAGS) $(CF_PROG2) $(LIBS) $(ARCH) -o $(PREF2)_opt_arch

$(PROG3_EXECS): $(P3)
	@echo "Program 3"
	$(CC) $(P3) $(CFLAGS) $(LIBS) -o $(PREF3)_norm
	$(CC) $(P3) $(CFLAGS) $(LIBS) $(ARCH) -o $(PREF3)_norm_arch
	$(CC) $(P3) $(CFLAGS) $(CF_PROG3) $(LIBS) -o $(PREF3)_opt
	$(CC) $(P3) $(CFLAGS) $(CF_PROG3) $(LIBS) $(ARCH) -o $(PREF3)_opt_arch

$(PROG4_EXECS): $(P4)
	@echo "Program 4"
	$(CC) $(P4) $(CFLAGS) $(LIBS) -o $(PREF4)_norm
	$(CC) $(P4) $(CFLAGS) $(LIBS) $(ARCH) -o $(PREF4)_norm_arch
	$(CC) $(P4) $(CFLAGS) $(CF_PROG4) $(LIBS) -o $(PREF4)_opt
	$(CC) $(P4) $(CFLAGS) $(CF_PROG4) $(LIBS) $(ARCH) -o $(PREF4)_opt_arch

.PHONY: clean
clean:
	rm $(EXECS)

.PHONY: test
test: $(EXECS)
	@echo "Testing all executable programs. Program names should be self-explaining."
	@for E in $(EXECS); do echo "\t" $$E ":"; $(TEST) ./$$E ; done
