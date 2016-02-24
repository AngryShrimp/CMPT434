/***
TCPServer.c

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
#include <sys/wait.h>
#include <signal.h>

#define PORT "30434"

#define BACKLOG 10
#define DICTIONARY_SIZE 10
#define DATA_SIZE 512

typedef struct keyedPair_t
{
	char key[11];
	char value[201];
} keyedPair;

keyedPair keyedDictionary[DICTIONARY_SIZE];

/*From Beej*/
void sigchld_handler(int s)
{
    /* waitpid() might overwrite errno, so we save and restore it: */
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

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

char* recvMsg(int fd, char* buffer)
{
	int numBytes = 0;
	if((numBytes = recv(fd, buffer, DATA_SIZE, 0)) == -1)
		perror("error in recv.\n");
	return buffer;
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

void getAll(int fd)
{
	int i;
	/*loop through all and return*/
	sendMsg(fd, "KEY\tVALUE\n=================\n");
	for(i = 0; i < DICTIONARY_SIZE; i++)
	{
		if(strcmp(keyedDictionary[i].key, "\0"))
		{
			sendMsg(fd, keyedDictionary[i].key);
			sendMsg(fd, "\t");
			sendMsg(fd, keyedDictionary[i].value);
			sendMsg(fd, "\n");
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
    int sockfd, new_fd;  /* listen on sock_fd, new connection on new_fd */
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; /* connector's address information */
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

	char* token;
	char buffer[200];
	/*Need a 'clone' of the buffer to mess around with, best not mess with the original buffer*/
	char bufferDup[200];
	char* tempKey;
	char* tempValue;
	char* tempGet;
	int i;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */

    /*Init keyed dictionary to all tombstones, A cemetary, if you will*/
    for(i = 0; i < DICTIONARY_SIZE; i++)
    {
    	keyedDictionary[i].key[0] = '\0';
    	keyedDictionary[i].value[0] = '\0';
    }

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    /* loop through all the results and bind to the first we can */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); /* all done with this structure */

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; /*/ reap all dead processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) 
    {  
    	/* main accept() loop */
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        sendMsg(new_fd, "\nConnected to the Keyed Dictionary Service.\nUse the help command for more information.\n\n:~>");
        recvMsg(new_fd, buffer);
        strcpy(bufferDup, buffer);

		
        /*Toeknize commands*/
        token = strtok(bufferDup, " ");
        /*Remove Newline*/
		token = strtok(token, "\n");

        /*printf("%s\n", token);*/

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
				sendMsg(new_fd, "Failed to Add, Check Syntaxing.\n");
			}
			else if(add(tempKey, tempValue) == -1)
			{
				sendMsg(new_fd, "Failed to add to Dictionary, full.\n");
			}
			else
			{
				sendMsg(new_fd, "Successfully Added!\n");
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
				sendMsg(new_fd, "ERROR: Key not found!\n");
			}
			else
			{
				sendMsg(new_fd, "Successfully Removed!\n");
			}
		}
		else if(strcmp(token, "getAll") == 0)
		{
			/*get ALL the stuff*/
			getAll(new_fd);
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
				sendMsg(new_fd, "ERROR: Key not found!\n");
			}
			else
			{
				sendMsg(new_fd, "The Value stored for key ");
				sendMsg(new_fd, tempKey);
				sendMsg(new_fd, " is: ");
				sendMsg(new_fd, tempGet);
				sendMsg(new_fd, "\n");
			}

		}
		else if(strcmp(token, "help") == 0)
		{
			/*help stuff*/
			sendMsg(new_fd, "HELP\nCommand\tSyntax\t\tDescription\nadd\t[Key] \"[Value]\" Add the key to the dictionary with the given value\nremove\t[key]\t\tRemoves the value with the given key from the dictionary\nget\t[key]\t\tRetrieves the value with the matching Key\ngetAll\tnone\t\tRetrieves all keys and values\n");
		}
		else
		{
			sendMsg(new_fd, "ERROR: Invalid Command/Syntax, please use \"help\" for commands.\n");
		}
		/*Clean up*/
		memset(buffer, 0, sizeof buffer);
		memset(bufferDup, 0, sizeof bufferDup);
        close(new_fd);
    }
    return 0;
}
