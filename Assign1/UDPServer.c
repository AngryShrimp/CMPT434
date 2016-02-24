/***
UDPServer.c

Keenan Johnstone - 11119412 - kbj182

Februray 3rd, 2016
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

#define BACKLOG 10
#define DICTIONARY_SIZE 10
#define DATA_SIZE 512
#define MAX_SEND_DATA 200

typedef struct keyedPair_t
{
	char key[11];
	char value[201];
} keyedPair;

keyedPair keyedDictionary[DICTIONARY_SIZE];


/*From Beej*/
/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

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
Function add

Takes and adds 'value' to a key dictionary with a 'key'

inputs:		key 	-The key for the value to be added to the dictionary
			value 	-The value to be put in the dictionary

returns:	0 		-On success
			-1 		-On failure

*/
int add(char* key, char* value)
{
	int i;
	char* searchKey;
	searchKey = strtok(key, " ");

	/*printf("SKey:%s:\n", searchKey);*/
	/*Loop through all keyed pairs looking for the first empty spot*/
	for(i = 0; i < DICTIONARY_SIZE; i++)
	{
		if(keyedDictionary[i].key[0] == '\0')
		{
			/*Need Str cpy other methods caused fun issues*/
			strcpy(keyedDictionary[i].key, searchKey);
			strcpy(keyedDictionary[i].value, value);
			return 0;
		}
	}
	/*REsults in a failure if this is reached, its full*/
	return -1;
}


/*
Function getValue

return value from matching (key, value) pair, if any

inputs:		key 	-The key to be searched  for in the dictionary

returns 	The value associated with the key

*/
char* getValue(char* key)
{
	int i;
	/*Loop through all pairs until a match is found*/
	for(i = 0; i < DICTIONARY_SIZE; i++)
	{
		if(strcmp(keyedDictionary[i].key, key) == 0)
		{
			/*MATCH*/
			return keyedDictionary[i].value;
		}
	}
	return NULL;
}

/*
Function getAll

inputs:		fd  	-A file descriptor 

returns 	-the entire dictionary of keyed values, in a human readabvle format.
*/

void getAll(int fd, char* s, struct sockaddr_storage addr, socklen_t addr_len)
{
	int i;
	/*loop through all and return*/
	sendMsg(fd, "KEY\tVALUE\n=================\n", addr, addr_len);
	for(i = 0; i < DICTIONARY_SIZE; i++)
	{
		if(strcmp(keyedDictionary[i].key, "\0"))
		{
			strcat(s, keyedDictionary[i].key);
			strcat(s, "\t");
			strcat(s, keyedDictionary[i].value);
			strcat(s, "\n");
			sendMsg(fd, s, addr, addr_len);
		}
	}
	return;
}

/*
Function remove

inputs 		-key 	-The ke for the value to be removed 

returns 	0 		if successful
			-1 		if failure
*/

int removeValue(char* key)
{
	int i;
	
	/*loop through all looking for key, then set to tombstone*/
	for(i = 0; i < DICTIONARY_SIZE; i++)
	{
		/*printf("MKey:%s:\n", keyedDictionary[i].key);*/
		if(strcmp(keyedDictionary[i].key, key) == 0)
		{
			/*MATCH*/
			/*printf("%s\n", "MATCH");*/
			keyedDictionary[i].key[0] = '\0';
			keyedDictionary[i].value[0] = '\0';
			return 0;
		}
	}
	return -1;
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
    char* token;

    char* tempKey;
    
    char* tempValue;
    char* tempGet;
    int pairCounter;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; /* set to AF_INET to force IPv4 */
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */
    numbytes = 0;
	/*Init pairs*/
	for(pairCounter = 0; pairCounter < DICTIONARY_SIZE; pairCounter++)
	{
		keyedDictionary[pairCounter].key[0] = '\0';
		keyedDictionary[pairCounter].value[0] = '\0';
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
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("listener: waiting to recvfrom...\n");
    while(1)
    {
        addr_len = sizeof addr;
        if ((numbytes = recvfrom(sockfd, buffer, MAX_SEND_DATA-1 , 0,
            (struct sockaddr *)&addr, &addr_len)) == -1) 
		{
            perror("recvfrom");
            exit(1);
        }

        printf("listener: got packet from %s\n",
            inet_ntop(addr.ss_family,
                get_in_addr((struct sockaddr *)&addr),
                s, sizeof s));
        printf("listener: packet is %d bytes long\n", numbytes);
        buffer[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buffer);


        strcpy(bufferDup, buffer);
        token = strtok(bufferDup, " ");

		if(strcmp(token, "add") == 0)
		{
			/*Add stuff*/
			/*tokenize the input*/
			strcpy(bufferDup, buffer);
			token = strtok(bufferDup, " ");
			token = strtok(NULL, " ");
			tempKey = strtok(token, " ");
			/*Copy the original buffer again*/
			strcpy(bufferDup, buffer);
			token = strtok(bufferDup, "\"");
			token = strtok(NULL, "\"");
			tempValue = token;
			if(token == NULL)
			{

				sendMsg(sockfd, "Failed to Add, Check Syntaxing.\n", addr, addr_len);
			}
			else if(add(tempKey, tempValue) == -1)
			{
				sendMsg(sockfd, "Failed to add to Dictionary, full.\n", addr, addr_len);
			}
			else
			{
				sendMsg(sockfd, "Successfully Added!\n", addr, addr_len);
			}

		}
		else if(strcmp(token, "remove") == 0)
		{
			/*remove stuff*/
			strcpy(bufferDup, buffer);
			token = strtok(bufferDup, " ");
			token = strtok(NULL, "\n");
			/*printf("%s\n", token);*/
			tempKey = token;
			if(removeValue(tempKey) == -1)
			{
				sendMsg(sockfd, "ERROR: Key not found!\n", addr, addr_len);
			}
			else
			{
				sendMsg(sockfd, "Successfully Removed!\n", addr, addr_len);
			}
		}
		else if(strcmp(token, "getAll") == 0)
		{
			/*get ALL the stuff*/
			getAll(sockfd, buffer, addr, addr_len);
		}
		else if(strcmp(token, "get") == 0)
		{
			/*get stuff*/
			strcpy(bufferDup, buffer);
			token = strtok(bufferDup, " ");
			token = strtok(NULL, "\n");
			tempKey = token;

			if((tempGet = getValue(tempKey)) == NULL)
			{
				sendMsg(sockfd, "ERROR: Key not found!\n", addr, addr_len);
			}
			else
			{
				sendMsg(sockfd, "The Value stored for key ", addr, addr_len);
				sendMsg(sockfd, tempKey, addr, addr_len);
				sendMsg(sockfd, " is: ", addr, addr_len);
				sendMsg(sockfd, tempGet, addr, addr_len);
				sendMsg(sockfd, "\n", addr, addr_len);
			}

		}
		else if(strcmp(token, "help") == 0)
		{
			/*help stuff*/
			sendMsg(sockfd, "HELP\nCommand\tSyntax\t\tDescription\nadd\t[Key] \"[Value]\" Add the key to the dictionary with the given value\nremove\t[key]\t\tRemoves the value with the given key from the dictionary\nget\t[key]\t\tRetrieves the value with the matching Key\ngetAll\tnone\t\tRetrieves all keys and values\n", addr, addr_len);
		}
		else
		{
			sendMsg(sockfd, "ERROR: Invalid Command/Syntax, please use \"help\" for commands.\n", addr, addr_len);
		}
		/*Clean up*/
		memset(buffer, 0, sizeof buffer);
		memset(bufferDup, 0, sizeof bufferDup);
        
    }
    close(sockfd);
    return 0;
}
