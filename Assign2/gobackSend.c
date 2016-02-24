/***
gobackSend.c
Keenan Johnstone - 11119412 - kbj182

March 2nd, 2016
**/

/**
TODO:


**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "30434"

#define MAX_SEND_DATA 200

/*
Functions for the easy sending and recieving of file descriptors
*/
int sendMsg(int fd, char s[MAX_SEND_DATA], struct sockaddr_storage addr, socklen_t addr_len)
{
	if (sendto(fd, s, strlen(s), 0, (struct sockaddr *)&addr, addr_len) == -1) {
		perror("send");
		return -1;
	}
	return 0;
}
/*
main:

Runs the server. Taken from: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver As recomended 

returns	 	EXIT_FAILURE on failure
*/
int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char buffer[MAX_SEND_DATA];
    char bufferDup[MAX_SEND_DATA];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; /* set to AF_INET to force IPv4 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */
    numbytes = 0;

	/*From Beej*/
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    /* loop through all the results and bind to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("Sender: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Sender: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Sender: failed to bind socket\n");
        return 2;
    }

    /*TODO: Get messages from user, about 10 to send*/

    /*TODO: Send message+header one at a time and resend any bad messages*/

    freeaddrinfo(servinfo);

    printf("Sender: waiting to recvfrom...\n");
    while(1)
    {
        /*Go Back and Slide shit here*/
    }
    close(sockfd);
    return 0;
}
