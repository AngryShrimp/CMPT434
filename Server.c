/***
Server.c

Keenan Johnstone - 11119412 - kbj182

January 15th, 2016
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define PORT "30434"

typedef struct _keyedPair_
{
	char[11] key;
	char[201] value;
} keyedPair;

keyedPair[100] keyedDictionary;

/*
Function add

Takes and adds 'value' to a key dictionary with a 'key'

inputs:		key 	-The key for the value to be added to the dictionary
			value 	-The value to be put in the dictionary

returns:	0 		-On success
			-1 		-On failure

*/
int add(char[11] key, char[201] value)
{
	return 0;
}


/*
Function getValue

return value from matching (key, value) pair, if any

inputs:		key 	-The key to be searched  for in the dictionary

returns 	The value associated with the key

*/
char* getValue(key[11])
{
	return NULL;
}

/*
Function getAll

returns 	-the entire dictionary of keyed values, in a human readabvle format.
*/

keyedPair getAll()
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

int remove(char[11] key)
{
	return 0;
}