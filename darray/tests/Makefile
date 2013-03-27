CC=cc
CFLAGS=-g -Wall -Wextra -static
LIBS=-lcunit -lpthread -lm
STD=-std=gnu99 -D_GNU_SOURCE
HEADER_DIR=-I..

testsuits = darray_normal_test darray_memory_leak_test

all: $(testsuits)

darray_normal_test: darray_normal_test.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) darray_normal_test.c $(LIBS) -o darray_normal_test

darray_memory_leak_test: darray_memory_leak_test.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) darray_memory_leak_test.c $(LIBS) -o darray_memory_leak_test

clean:
	rm -f *.o a.out $(testsuits) *core
