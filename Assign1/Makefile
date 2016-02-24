
CC=gcc

CFLAGS=-Wall -g -pedantic -Wall

all: TCPServer TCPProxy UDPServer

TCPServer: TCPServer.c
	$(CC) $(CFLAGS) TCPServer.c -o TCPServer

TCPProxy: TCPProxy.c
	$(CC) $(CFLAGS) TCPProxy.c -o TCPProxy

UDPServer: UDPServer.c
	$(CC) $(CFLAGS) UDPServer.c -o UDPServer

#UDPProxy: UDPProxy.c
#	$(CC) $(CFLAGS) UDPProxy.c -o UDPProxy
	

#Cleanup.
clean:
	rm -f *.o TCPServer
	rm -f *.o TCPProxy
	rm -f *.o UDPServer
#	rm -f *.o UDPProxy
	
