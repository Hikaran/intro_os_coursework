CC=gcc
CFLAGS=-std=c99 -pthread
DBFLAGS=-g

all: server client

client: client.c *.h
	$(CC) $(CFLAGS) -o client client.c

server: server.c *.h
	$(CC) $(CFLAGS) -o server server.c
