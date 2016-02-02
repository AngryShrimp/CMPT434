/***
Server.c

Keenan Johnstone - 11119412 - kbj182

January 15th, 2016
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

typedef struct keyedPair_t
{
	char key[11];
	char value[201];
} keyedPair;

keyedPair keyedDictionary[100];


void sigchld_handler(int s)
{
    /* waitpid() might overwrite errno, so we save and restore it: */
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
Function add

Takes and adds 'value' to a key dictionary with a 'key'

inputs:		key 	-The key for the value to be added to the dictionary
			value 	-The value to be put in the dictionary

returns:	0 		-On success
			-1 		-On failure

*/
int add(char key[11], char value[201])
{
	return 0;
}


/*
Function getValue

return value from matching (key, value) pair, if any

inputs:		key 	-The key to be searched  for in the dictionary

returns 	The value associated with the key

*/
char* getValue(char key[11])
{
	return NULL;
}

/*
Function getAll

returns 	-the entire dictionary of keyed values, in a human readabvle format.
*/

keyedPair* getAll()
{
	return keyedDictionary;
}

/*
Function remove

inputs 		-key 	-The ke for the value to be removed 

returns 	0 		if successful
			1 		if no key matching
			-1 		if failure
*/

int removeValue(char key[11])
{
	return 0;
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

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */

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

    while(1) {  /* main accept() loop */
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

        if (!fork()) { /* this is the child process */
            close(sockfd); /* child doesn't need the listener */
			/*HAVE TO DO SWITCH STATEMENTS HERE*/
            exit(0);
        }
        close(new_fd);  /* parent doesn't need this */
    }

    return 0;
}