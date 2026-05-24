# makefile for thread pool
#

CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -pthread

all: client.o threadpool.o
	$(CC) $(CFLAGS) -o example client.o threadpool.o

client.o: client.c threadpool.h
	$(CC) $(CFLAGS) -c client.c

threadpool.o: threadpool.c threadpool.h
	$(CC) $(CFLAGS) -c threadpool.c

clean:
	rm -rf *.o
	rm -rf example
