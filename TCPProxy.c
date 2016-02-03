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
	return 0;
}

int recvMsg(int fd, char* buffer)
{
	int numBytes = 0;
	if((numBytes = recv(fd, buffer, DATA_SIZE-1, 0)) == -1)
	{
		perror("error in recv.\n");
	}
	return numBytes;
}

int redirect(int from, int to)
{
	char buffer[DATA_SIZE];
	memset(buffer, 0, sizeof buffer);
	int lost_connection = 0;
	size_t bytes_read, bytes_written;
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
    memset(buffer, 0, sizeof buffer);
    return lost_connection;
}

int main(int argc, char **argv)
{
	int sock;
	struct addrinfo hints, *results;
	char *host, *port;
	struct sockaddr_in addr;
	socklen_t size;
	
	int client;
	int server = -1;
	int lost_connection = 0;
    
	int sockets_max;

	fd_set set;

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
            
            /* Get the address info */
			memset(&hints, 0, sizeof hints);
			memset(&results, 0, sizeof results);
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
			if (connect(server, results->ai_addr, results->ai_addrlen) == -1) {
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
    }

    close(sock);

    return 0;
}