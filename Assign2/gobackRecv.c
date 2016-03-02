/***
gobackRecv.c
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
#include <stdbool.h>
#include <time.h>

#define PORT "30434"

#define MAX_SEND_DATA 200
#define MAX_MESSAGE_LENGTH 128
#define MAX_PACKETS 100


typedef struct _PACKET_
{
    int packetNumber;
    char message[MAX_MESSAGE_LENGTH];
    bool ack;
} packet;

packet packetArray[MAX_PACKETS];
bool packetAck[MAX_PACKETS];

/*
Function isRandomError

inputs 		p 			the probability that there is an error
						set to 0 for no errors
						set to negative numbers or greater than or equal to 100 for always errors

returns 	false 		if no error
			true 		if there is an error
*/
bool isRandomError(int p)
{
	int r;

	if(p == 0)
		return false;
	if(p >= 100 || p < 0)
		return true;

	r = rand() % 100;
	/*Error*/
	if(r <= p)
		return true;
	else
		return false;
}

/*
main:

Runs the server. Taken from: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver As recomended 

returns	 	EXIT_FAILURE on failure
*/
int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    int errorProbability;
	size_t size;
    char *response;
    packet currentPacket;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int i;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; /* set to AF_INET to force IPv4 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */
    numbytes = 0;
	response = malloc(sizeof(char*));

	if (argc != 2) {
        fprintf(stderr,"Incorrect Usage, use: gobackRecv error_probability\n");
        exit(1);
    }

    errorProbability = atoi(argv[1]);

    for (i = 0; i < MAX_PACKETS; i++)
    {
        packetAck[i] = false;
    }

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

    /*TODO: Get messages from sender, read out in order*/

    /*TODO: Let user inject errors at will*/

    freeaddrinfo(servinfo);

    printf("Recv: waiting to recvfrom...\n");
    while(1)
    {
    	addr_len = sizeof addr;
        if ((numbytes = recvfrom(sockfd, (char*)&currentPacket, sizeof(currentPacket), 0,(struct sockaddr *)&addr, &addr_len)) == -1) 
        {
        	perror("revfrom");
        	exit(1);
        }
        if(packetAck[currentPacket.packetNumber] == true)
        {
        	printf("Already Recieved Message:\n");
        	printf("---------------------------\n");
        	printf("#%d\t%sAck:%d\n", currentPacket.packetNumber, currentPacket.message, currentPacket.ack);
        	printf("---------------------------\n");
        	printf("Still good? (Y/N): ");
        }
        else
        {
        	printf("Message Recieved!\n");
        	printf("---------------------------\n");
        	printf("#%d\t%sAck:%d\n", currentPacket.packetNumber, currentPacket.message, currentPacket.ack);
        	printf("---------------------------\n");
        	printf("Was this recieved Correctly? (Y/N): ");
        }
        getline(&response, &size, stdin);

        if(response[0] == 'Y' || response[0] == 'y')
        {
        	/*If there is a randomm error*/
        	if(isRandomError(errorProbability) == true)
        	{
        		currentPacket.ack = false;
        		printf("====================\n");
        		printf("ERROR IN SENDING ACK\n");
	      		printf("====================\n");
        	}
        	/*Successfully sent ack*/
        	else
        	{
        		packetAck[currentPacket.packetNumber] = true;
        		currentPacket.ack = true;
        	}

        }
        else if(response[0] == 'N' || response[0] == 'n')
        {
        	currentPacket.ack = false;
        }
        else
        {
        	printf("Invalid Response, treating as a No.\n");
        	currentPacket.ack = false;
        }

        if((numbytes = sendto(sockfd, (char*)&currentPacket, sizeof(currentPacket), 0, (struct sockaddr*)&addr, addr_len) == -1))
        {
        	perror("sendto");
        	exit(1);
        }  
    }
    close(sockfd);
    return 0;
}
