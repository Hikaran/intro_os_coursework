CC=gcc
CFLAGS=-std=c99 -pthread -D_XOPEN_SOURCE=500
DBFLAGS=-g

make: votecounter test

votecounter: votecounter.c *.h
	$(CC) $(CFLAGS) votecounter.c -o votecounter

test: test.c *.h
	$(CC) $(CFLAGS) test_*.h test.c -o test

clean:
	rm votecounter test
