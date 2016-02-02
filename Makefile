
CC=gcc

CFLAGS=-Wall -g -pedantic 

all: TCPServer

TCPServer: TCPServer.c
	$(CC) $(CFLAGS) TCPServer.c -o TCPServer

#Cleanup.
clean:
	rm -f *.o TCPServer
	
