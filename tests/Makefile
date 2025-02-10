CC=gcc
CFLAGS=-I. -Wall
LIBS=-lcriterion -lpthread

test: tests/test_hybrid_malloc.c my_malloc.c
    $(CC) $(CFLAGS) -o test_hybrid tests/test_hybrid_malloc.c my_malloc.c $(LIBS)
    ./test_hybrid --verbose