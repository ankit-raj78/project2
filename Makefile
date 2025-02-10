CC=gcc
CFLAGS=-I. -Wall
LIBS=-lcriterion -lpthread

all: lib test

lib: my_malloc.o
	$(CC) $(CFLAGS) -shared -o libmymalloc.so my_malloc.o

test: tests/test_hybrid_malloc.c my_malloc.c
	$(CC) $(CFLAGS) -o test_hybrid tests/test_hybrid_malloc.c my_malloc.c $(LIBS)
	./test_hybrid --verbose

test_malloc_unity.o: tests/test_malloc_unity.c
	$(CC) $(CFLAGS) -c tests/test_malloc_unity.c

my_malloc.o: my_malloc.c
	$(CC) $(CFLAGS) -fPIC -c my_malloc.c

clean:
	rm -f *.o *.so test_malloc_unity

.PHONY: all clean
