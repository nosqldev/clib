CC=cc
STD=-std=gnu99 -D_GNU_SOURCE
CFLAGS=-g -Wall -Wextra
LIB=-lpthread
HEADER_DIR=-I../ -I../../assertest/
HEADER_FILE=../cexception.h

tests= exception_test

all: $(tests)

exception_test: $(HEADER_FILE) exception_test.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) exception_test.c $(LIB) -o exception_test

.PHONY: clean
clean:
	rm -f *.o *.out $(tests)
