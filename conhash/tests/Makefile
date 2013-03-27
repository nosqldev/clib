CC=cc
STD=-std=gnu99 -D_GNU_SOURCE
CFLAGS=-g -Wall -Wextra -static
LIB=-lpthread -lm -lcunit
HEADER_DIR=-I.. -I../../timeutil/
HEADER_FILE=../cfhash.h

libs = nshash.o rwlock.o
testsuits = $(libs) puttest gettest strrp cleartest deltest mtrp stp

all: $(libs) $(testsuits)

nshash.o:
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -c ../nshash.c -o nshash.o

rwlock.o:
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -c ../rwlock.c -o rwlock.o

puttest: $(HEADER_FILE) puttest.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) puttest.c $(libs) $(LIB) -o puttest

gettest: $(HEADER_FILE) gettest.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) gettest.c $(libs) $(LIB) -o gettest

strrp: $(HEADER_FILE) single_thread_rand_read_perf.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -O2 single_thread_rand_read_perf.c $(libs) $(LIB) -o strrp

cleartest: $(HEADER_FILE) cleartest.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) cleartest.c $(libs) $(LIB) -o cleartest

deltest: $(HEADER_FILE) deltest.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) deltest.c $(libs) $(LIB) -o deltest

mtrp: $(HEADER_FILE) multi_thread_read_perf.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -O2 multi_thread_read_perf.c $(libs) $(LIB) -o mtrp

stp: $(HEADER_FILE) single_thread_press.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -O2 single_thread_press.c $(libs) $(LIB) -o stp

clean:
	-rm -f *core *.c_final.c *.c_out.c $(testsuits)
