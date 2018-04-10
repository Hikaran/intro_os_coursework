CC=gcc
CFLAGS=-std=c99 -pthread
DBFLAGS=-g

make: test

test: test.c *.h
	$(CC) $(CFLAGS) test_*.h test.c -o test

clean:
	rm test
