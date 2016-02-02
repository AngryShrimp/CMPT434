
CC=gcc

CFLAGS=-Wall -g -pedantic -Werror

all: Server

server: Server.c
	$(CC) $(CFLAGS) Server.c -o Server

#Cleanup.
clean:
	rm -f *.o Server
	
