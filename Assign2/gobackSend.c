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
#include <stdbool.h>
#include <sys/poll.h>

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
int packetOffset = 0;

void getMessages()
{
    int i;
    int numMessages;
    char* m;
    size_t size;
    m = malloc(sizeof(char*));
    printf("How many messages would you like to enter? (NOTE: max is: %d)\n", MAX_PACKETS);
    getline(&m, &size, stdin);
    numMessages = atoi(m);
    printf("Please Enter messages %d to %d, no longer than %d and seperated by newlines.\n", packetOffset, (packetOffset + numMessages - 1), MAX_MESSAGE_LENGTH);
    for(i = 0 + packetOffset; i < numMessages + packetOffset; i++)
    {
        if(i >= MAX_PACKETS)
        {
            printf("MAX MESSAGES REACKED\n");
            break;
        }
        printf("Message #%d: ", i);
        getline(&m, &size, stdin);
        strcpy(packetArray[i].message, m);
        packetArray[i].packetNumber = i;
        packetArray[i].ack = false;
    }
    

    /*Print all for debugging.*/
   /* for(i = 0; i < numMessages; i++)
    {
        printf("Printing all Messages!\n");
        printf("Message %d: %s\n", packetArray[i + packetOffset].packetNumber, packetArray[i + packetOffset].message);
    }*/
    packetOffset = i;
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
    int recvPoll;
    int numbytes;
    int i; /*Counter*/
    int timeout, windowSize, windowStart;
    struct pollfd clientPoll;
    packet currentPacket;
    struct sockaddr_in clientAddr;
    socklen_t addr_len;
    bool windowIsOK;

    
    memset(&clientAddr, 0, sizeof(clientAddr));    

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; /* set to AF_INET to force IPv4 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */
    numbytes = 0;

    if (argc != 5) {
        fprintf(stderr,"Incorrect Usage, use: gobackSend host port timeout(ms) windowSize \n");
        exit(1);
    }

    windowStart = 0;
    timeout = atoi(argv[3]);
    windowSize = atoi(argv[4]);
    

	/*From Beej*/
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    /* loop through all the results and bind to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("Sender: socket");
            continue;
        }
        break;
    }
    clientPoll.fd = sockfd;
    clientPoll.events = POLLIN;

    if (p == NULL) {
        fprintf(stderr, "Sender: failed to bind socket\n");
        return 2;
    }

    /*TODO: Get messages from user, about 10 to send*/

    /*TODO: Send message+header one at a time and resend any bad messages*/
    freeaddrinfo(servinfo);

    /*Connection Made, grab initial messages*/
    getMessages();
    while(1)
    {
        /*Reset for when window size is smaller than what the user defines*/
        windowSize = atoi(argv[4]);

        addr_len = sizeof(clientAddr);
        if(windowStart >= packetOffset)
        {
            /*Get more Messages!*/
            printf("More Messages Required, please enter more messages!\n");
            getMessages();

        }
        /*If our number of packets to be sent is less than our windowsize, adjust our current window*/
        if(packetOffset - windowStart < windowSize)
        {
            printf("Number of Packets is smaller than window, sending incomplete window\n");
            windowSize = packetOffset - windowStart;    
        }
        printf("Sending Window, with packet %d to %d \n", windowStart, windowStart + windowSize - 1);
        /*Send Window*/
        for(i = windowStart; i < (windowStart + windowSize); i++)
        {

            /*Interesting bug, it wont print this until later unless I use a \n character.*/
            printf("Sending Message #%d...\n", i);
            if((numbytes = sendto(sockfd, (char*)&packetArray[i], sizeof(packetArray[i]), 0, p->ai_addr, p->ai_addrlen)) == -1)
            {
                perror("sendto");
                exit(1);
            } 
            
            /*Poll Recv for Response on timeout*/
            recvPoll = poll(&clientPoll, 1, timeout);
            /*Timeout*/
            if(recvPoll == 0)
            {
                printf("Timed Out.\n");
            }
            /*Error*/
            else if(recvPoll == -1)
            {
                perror("POLLIN");
                exit(1);
            }
            /*Else we gucci*/
            else
            {
                printf("Response!\n");
                if ((numbytes = recvfrom(sockfd, (char*)&currentPacket, sizeof(currentPacket), 0, (struct sockaddr *)p->ai_addr, (socklen_t *)&addr_len)) == -1) 
                {
                    perror("revfrom");
                    exit(1);
                }
                printf("---------------------------\n");
                printf("#%d\t %sAck:%d\n", currentPacket.packetNumber, currentPacket.message, currentPacket.ack);
                printf("---------------------------\n");

                /*Set ackArray to match what was recieved*/
                packetAck[currentPacket.packetNumber] = currentPacket.ack;

            }
        }
        windowIsOK = false;
        for(i = windowStart; i < (windowStart + windowSize); i++)
        {
            if(packetAck[i] == true)
                windowIsOK = true;
            else
            {
                windowIsOK = false;
                break;
            }
        }
        if(windowIsOK == true)
        {
            printf("Window is okay! Sending next window!\n");
            windowStart += windowSize;
        }
        else
        {
            printf("Window is NOT okay. Resending.\n");
            for(i = windowStart; i < (windowStart + windowSize); i++)
            {
                packetAck[i] = false;
            }
        }
        printf("====================\n");
        
    }
    
    close(sockfd);
    return 0;
}
