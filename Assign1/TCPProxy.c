/***
TCPProxy.c

Keenan Johnstone - 11119412 - kbj182

Februray 3rd, 2016
**/
#include <stdio.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#define BACKLOG  10
#define PORT "30435"
#define DATA_SIZE 512



/*
Functions for the easy sending and recieving of file descriptors
*/
int sendMsg(int fd, char* s)
{
	int numBytes;
	if((numBytes = send(fd, s, strlen(s), 0)) == -1)
	{
		perror("error in sending.\n");
		return -1;
	}
	return numBytes;
}

int recvMsg(int fd, char* buffer)
{
	int numBytes = 0;
	if((numBytes = recv(fd, buffer, DATA_SIZE-1, 0)) == -1)
	{
		perror("error in recv.\n");
	}
	buffer[numBytes] = '\0';
	return numBytes;
}

int redirect(int from, int to)
{
	char buffer[DATA_SIZE];
	
	int lost_connection;
	size_t bytes_read, bytes_written;
	lost_connection = 0;
	
	bytes_read = recvMsg(from, buffer);
	if (bytes_read == 0) 
	{
		/*Connection Lost: cleanup*/
		lost_connection = 1;
		memset(buffer, 0, sizeof buffer);
	}
	else 
	{
		bytes_written = sendMsg(to, buffer);
		if (bytes_written == -1) 
		{
			lost_connection = 1;
			memset(buffer, 0, sizeof buffer);
        }

    }
    return lost_connection;
}

void helper(int client, char *host, char *port)
{
	struct addrinfo hints, *results;
	int server;
	int lost_connection;
	
	fd_set set;
	int sockets_max;
	
	lost_connection = 0;
	server = -1;
	
	/* Get the address info */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(host, port, &hints, &results) != 0) 
	{
		perror("getaddrinfo");
		close(client);
		exit(1);
	}

	/* Create the socket */
	server = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	if (server == -1) 
	{
		perror("socket");
  		close(client);
		exit(1);
	}

	/* Connect to the host */
	if (connect(server, results->ai_addr, results->ai_addrlen) == -1) 
	{
		perror("connect");
		close(client);
		exit(1);
	}

	if (client > server) 
	{
		sockets_max = client;
	}
	else 
	{
		sockets_max = server;
	}

	/* Main redirect loop */
	while (lost_connection == 0) 
	{
		FD_ZERO(&set);
		FD_SET(client, &set);
		FD_SET(server, &set);
		if (select(sockets_max + 1, &set, NULL, NULL, NULL) == -1) 
		{
			perror("select");
			break;
		}
		if (FD_ISSET(client, &set)) 
		{
			lost_connection = redirect(client, server);
		}
		if (FD_ISSET(server, &set)) 
		{
			lost_connection = redirect(server, client);
		}
	}
	close(server);
	close(client);
}


int main(int argc, char **argv)
{
	int sock;
	struct addrinfo hints, *results;
	struct sockaddr_in addr;
	socklen_t size;
	
	char *host, *port;
	
	int client;

	int reuseaddr;
	
	reuseaddr = 1;
    /* Get the server host and port from the command line */
    if (argc < 2) 
    {
        fprintf(stderr, "Syntax: %s [Host of Server] [Port of Server]\n", argv[0]);
        exit(1);
    }
    host = argv[1];
    port = argv[2];

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    /*Use our own IP*/
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, PORT, &hints, &results) != 0) 
    {
        perror("getaddrinfo");
        exit(1);
    }

    /* Create the socket */
    sock = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if (sock == -1) {
        perror("socket");
        freeaddrinfo(results);
        exit(1);
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        freeaddrinfo(results);
        exit(1);
    }

    /* Bind to the address */
    if (bind(sock, results->ai_addr, results->ai_addrlen) == -1) {
        perror("bind");
        freeaddrinfo(results);
        exit(1);
    }

    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        freeaddrinfo(results);
        exit(1);
    }

    freeaddrinfo(results);

    /* Ignore broken pipe signal */
    signal(SIGPIPE, SIG_IGN);

    /* Main loop */
    while (1) {
        size = sizeof addr;
        
        client = accept(sock, (struct sockaddr*)&addr, &size);
        if (client == -1) 
        {
            perror("accept");
        }
        else 
        {
            printf("Connection from %s\n", inet_ntoa(addr.sin_addr));
			helper(client, host, port);
		}    
            
    }

    close(sock);

    return 0;
}