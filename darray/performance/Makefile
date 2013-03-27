CC=cc
CFLAGS=-g -Wall -Wextra -static
LIBS=-lcunit -lpthread -lm
STD=-std=gnu99 -D_GNU_SOURCE
HEADER_DIR=-I.. -I../../timeutil

program = random_access_darray random_access_darray_mul_thread

all: $(program) pos

random_access_darray: random_access_darray.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) random_access_darray.c $(LIBS) -o random_access_darray

random_access_darray_mul_thread: random_access_darray.c
	$(CC) $(STD) $(CFLAGS) $(HEADER_DIR) -DMUL_THREAD random_access_darray.c $(LIBS) -o random_access_darray_mul_thread

pos:
	./gen_pos_file.pl 10000

clean:
	rm -f *.o a.out $(program) *core pos
