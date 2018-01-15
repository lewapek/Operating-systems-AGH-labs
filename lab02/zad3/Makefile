CC = gcc
CFLAGS = -Wall -std=gnu99 -I.
LINKFLAGS = -Wl,-rpath=/usr/local/lib
NORM_LIBS = -lrt
LIBS = $(NORM_LIBS)
STATIC = -static
DYNAMIC = -dynamic
LOADED_DYNAMICALLY = -ldl -D DYNAMIC
SRCS = test_mem_managementc test_matrixesc

all: $(SRCS)

$(SRCS): %c:%.c
	$(CC) $(CFLAGS) $< -c -o $*.o
	$(CC) $(CFLAGS) stat.c -c -o stat.o
	$(CC) $*.o stat.o $(LIBS) $(LINKFLAGS) $(STATIC) -o $*_static
	$(CC) $*.o stat.o $(LIBS) $(LINKFLAGS) $(DYNAMIC) -o $*_dynamic
	$(CC) $*.o stat.o $(NORM_LIBS) $(LINKFLAGS) $(LOADED_DYNAMICALLY) -lmem_management -lmatrixes -o $*_dl

test_mem_management%: LIBS = $(NORM_LIBS) -lmem_management
test_matrixes%: LIBS = $(NORM_LIBS) -lmem_management -lmatrixes

.PHONY: clean
clean:
	@rm -Rf *.o
	@rm -Rf test_mem_management_static test_mem_management_dynamic test_mem_management_dl test_matrixes_static test_matrixes_dynamic test_matrixes_dl
	@rm *.results

.PHONY: run
run:
	@echo "\ttest_mem_management_static"
	./test_mem_management_static > test_mem_management_static.results
	@echo "\ttest_mem_management_dynamic"
	./test_mem_management_dynamic > test_mem_management_dynamic.results
	@echo "\ttest_mem_management_dl"
	./test_mem_management_dl > test_mem_management_dl.results
	@echo "\ttest_matrixes_static"
	./test_matrixes_static "input.matrix" "+" > test_matrixes_static.results
	@echo "\ttest_matrixes_dynamic"
	./test_matrixes_dynamic "input.matrix" "+" > test_matrixes_dynamic.results
	@echo "\ttest_matrixes_dl"
	./test_matrixes_dl "input.matrix" "+" > test_matrixes_dl.results
