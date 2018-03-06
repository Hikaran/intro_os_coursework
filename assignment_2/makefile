CC=gcc
CFLAGS=-std=c99
DBFLAGS=-g

make: Leaf_Counter Aggregate_Votes Vote_Counter

Leaf_Counter: Leaf_Counter.c util.h
	$(CC) $(CFLAGS) Leaf_Counter.c -o Leaf_Counter

Aggregate_Votes: Aggregate_Votes.c util.h votes.c
	$(CC) $(CFLAGS) Aggregate_Votes.c -o Aggregate_Votes

Vote_Counter: Vote_Counter.c util.h
	$(CC) $(CFLAGS) Vote_Counter.c -o Vote_Counter

clean:
	rm Leaf_Counter Aggregate_Votes Vote_Counter
