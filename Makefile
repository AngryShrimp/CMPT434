
CC=gcc

CFLAGS=-Wall -g -pedantic 

all: TCPServer TCPProxy UDPServer

TCPServer: TCPServer.c
	$(CC) $(CFLAGS) TCPServer.c -o TCPServer

TCPProxy: TCPProxy.c
	$(CC) $(CFLAGS) TCPProxy.c -o TCPProxy

#UDPServer: UDPServer.c
#	$(CC) $(CFLAGS) UDPServer.c -o UDPServer

#Cleanup.
clean:
	rm -f *.o TCPServer
	rm -f *.o TCPProxy
	rm -f *.o UDPServer
	
