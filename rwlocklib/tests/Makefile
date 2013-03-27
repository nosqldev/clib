all:
	cc -Wall -Wextra -c -g -I.. ../rwlock.c -o rwlock.o
	cc -Wall -Wextra -c -g -I../../slab/ -I.. testmain.c -o testmain.o
	cc -Wall -Wextra -c -I ../../slab/ ../../slab/slab.c -o slab.o
	cc -Wall -Wextra -g -std=gnu99 -DNEED_DEBUG -D_GNU_SOURCE testmain.o rwlock.o slab.o -lpthread -o test

clean:
	rm -rf *.o a.out test
