CC=gcc
CFLAGS=-std=c99 -pthread
DBFLAGS=-g

make: test

test: test.c test_*.h
	$(CC) $(CFLAGS) test_*.h test.c -o test

clean:
	rm test
