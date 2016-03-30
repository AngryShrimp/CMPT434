/****************
Keenan Johnstone
11119412 - kbj182
Mar 30 2016
Assignment 3
****************/

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
#include <time.h>

/*Zone Dimensions*/
#define ZONE_WIDTH 20
#define ZONE_LENGTH 20

/*Maximum number of nodes*/
#define MAX_NODES 10

/*The port that the nodes connect to*/
#define PORT 30434

/*Node structure for storing information on the location of the node and the nodes name
  Also includes a list of all the connections*/
typedef struct _SENSOR_NODE_
{
	char name[5];
	int x_pos;
	int y_pos;
	struct sensor_node* connected_nodes[MAX_NODES];

} sensor_node;

/*For displaying the locations of all the nodes*/
char** map;

/*List of all nodes and the base*/
sensor_node* node_list;
sensor_node base;

/*Also need listeners and their ports*/
int listeners[MAX_NODES+1];
int listener_ports[MAX_NODES+1];


/*From beej's network guide*/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*From beej's network guide*/
void sigchld_handler(int s)
{
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

/*
Function: init_map
Clear the map with underscores

return:
		0: 	Success
		-1:	Error
*/
int init_map()
{
	int i, j;
	
	/*Init the 2-D character array*/
	map = (char**)malloc(sizeof(char*)*ZONE_LENGTH);
	for(i = 0; i < ZONE_LENGTH; i++)
		map[i] = (char*)malloc(sizeof(char*)*ZONE_WIDTH);

	/*Set all characters to '_'*/
	for(i = 0; i < ZONE_LENGTH; i++)
	{
		for(j = 0; j < ZONE_WIDTH; j++)
			map[i][j] = '-';
	}
	return 0;
}
/*
Function: print_map
prints the map

input:
		N/A

return:
		N/A
*/
void print_map()
{
	int i,j;
	for(i = 0; i < ZONE_WIDTH; i++)
	{
		for(j = 0; j < ZONE_LENGTH; j++)
		{
			printf("%c", map[i][j]);
		}
		printf("\n");
	}
}


/*
Function: set_node_location
Set the location of the node randomly

input:
		the node to be placed

return:
		0: 	Success
		-1:	Error
*/
int set_node_location(sensor_node *n)
{

	int x;
	int y;
	x = rand() % (ZONE_WIDTH-1);
	y = rand() % (ZONE_LENGTH-1);
	if(x < 0)
		x = 0;
	if(y < 0)
		y = 0;

	n->x_pos = x;
	n->y_pos = y;
	
	map[y][x] = 'N';
	printf("x%d y%d\n", x, y);
	printf("%s: Location set!\n", n->name);
	return 0;
}

/*
Function: set_base_location
Set the location of the stationary node/base

return:
		0: 	Success
		-1:	Error
*/
int set_base_location()
{
	base.x_pos = ZONE_WIDTH/2;
	base.y_pos = ZONE_LENGTH/2;

	map[base.y_pos][base.x_pos] = '@';
	return 0;
}

/*
Function: move_node
Move the given node on the map with the given distance in a random direction
Has to not go out of bounds and 'bounce off of the walls' of the zone

input:
		n: 			the node to be placed
		distance:	how far the node will move (user defined)

return:
		0: 	Success
		-1:	Error
*/
int move_node(int distance, sensor_node* n)
{
	return 0;
}
/*
Function: node_init
init the node after forking. This means connecting to all other nodes

input:
		n: 			the node to be created
		distance:	how far the node will move (user defined)

return:
		N/A
*/
void node_init(int time_step, char* node_ID, int ID_num, sensor_node* n)
{
	/*Counter Variables*/
	int i;

	/*Each node needs a certain number of sockets for each connection*/
	int sock_fd[MAX_NODES+1], new_fd[MAX_NODES+1];

	/*Server crap*/
	int rv;
	struct addrinfo hints, *servinfo, *p;
	char port[5];
	char buf[5];
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr;
	socklen_t sock_size;

	strcpy(n->name, node_ID);


	/*For each other node...*/
	for(i = 1; i < MAX_NODES+1; i++)
	{
		/*if it's not the node we are on*/
		if(i != ID_num)
		{
			/*Setup lister for node*/
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE; /* use my IP*/

			/*convert port to string*/
			sprintf(port, "%d", listener_ports[i]);

			/*from beej*/
			if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) 
			{
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				return;
			}

			/*From beej's*/
			/* loop through all the results and bind to the first we can*/
			for(p = servinfo; p != NULL; p = p->ai_next) 
			{
				if ((sock_fd[i] = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) 
				{
					perror("server: socket");
					continue;
				}
				if(connect(sock_fd[i], p->ai_addr, p->ai_addrlen) == -1)
				{
					fprintf(stderr, "\nError in connecting!");
				}
				/*Connection*/
				break;
			}
			inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
			/*printf("Node %d: Connecting to %s\n", ID_num, s);*/

			sock_size = sizeof their_addr;
			new_fd[i] = accept(listeners[i], (struct sockaddr*)&their_addr, &sock_size);
			if(new_fd[i] == -1)
			{
				fprintf(stderr, "Error on accept. Socket: %d\n", new_fd[i]);
				perror("accept");
				continue;
			}
		}
	}
	printf("%s: All connected!\n", node_ID);

	freeaddrinfo(servinfo);
	/*Guts of the program here*/

	/*Place each node in the map*/
	set_node_location(n);

	/*print map for test*/

	exit(1);
}



/*Main*/
int main(int argv, char** argc)
{
	/*Server stuff*/
	struct addrinfo hints, *servinfo, *p;
	struct sigaction sa;
	int yes;
	int rv;
	char port[5];

	/*forking stuff*/
	int pid[MAX_NODES];
	char* buf;
	

	/*User set variables*/
	int num_time_steps;
	int distance;
	int transmission_range;

	/*Counter variables*/
	int i;
	int j;

	/*init variables*/
	node_list = malloc(sizeof(node_list));
	buf = malloc(sizeof(buf));

	/*Init port numbers*/
	for(i = 0; i < MAX_NODES+1; i++)
		listener_ports[i] = PORT+i;

	/*User input vairables*/
	if(argv != 4)
	{
		printf("Incorrect usage:\n\nFormat:\tmobile_sensor num_time_steps distance_traveled transmission_range\n\n");
		return EXIT_FAILURE;
	}
	num_time_steps = atoi(argc[1]);
	distance = atoi(argc[2]);
	transmission_range = atoi(argc[3]);


	/*get a random seed, comment out for no randomness*/
	srand(time(NULL));

	/*init the map*/
	init_map();

	/*Set base node location*/
	set_base_location();

	/*Server crap*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; /* use my IP*/
	for(i = 0; i < MAX_NODES+1; i++)
	{
		/*Convert int port numbers from before to strings!*/
		sprintf(port, "%d", listener_ports[i]);

		if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		/* loop through all the results and bind to the first we can*/
		for(p = servinfo; p != NULL; p = p->ai_next) 
		{
			if ((listeners[i] = socket(p->ai_family, p->ai_socktype,
					p->ai_protocol)) == -1) {
				perror("server: socket");
				continue;
			}

			if (setsockopt(listeners[i], SOL_SOCKET, SO_REUSEADDR, &yes,
					sizeof(int)) == -1) {
				perror("setsockopt");
				exit(1);
			}

			if (bind(listeners[i], p->ai_addr, p->ai_addrlen) == -1) 
			{
				close(listeners[i]);
				perror("server: bind");
				continue;
			}

			break;
		}

		freeaddrinfo(servinfo); /* all done with this structure*/

		if (p == NULL)  {
			fprintf(stderr, "server: failed to bind\n");
			exit(1);
		}

		if (listen(listeners[i], MAX_NODES+1) == -1) 
		{
			perror("listen");
			exit(1);
		}

		sa.sa_handler = sigchld_handler; /* reap all dead processes*/
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_RESTART;
		if (sigaction(SIGCHLD, &sa, NULL) == -1) 
		{
			perror("sigaction");
			exit(1);
		}
	}

	/*Now Fork all Nodes form parent process*/
	for(i = 0; i < MAX_NODES; i++)
	{
		pid[i] = fork();
		/*Failure to fork*/
		if(pid[i] < 0)
		{
			fprintf(stderr, "ERROR: Forks forking themsleves up.\n");
			break;
		}
		/*Child/Node stuff*/
		else if(pid[i] == 0)
		{
			if(i+1 >= 10)
				sprintf(buf, "N%d", i+1);
			else
				sprintf(buf, "N0%d", i+1);
			srand(time(NULL)*i);
			node_init(num_time_steps, buf, i+1, node_list);
		}
		else
		{
			/*CANT WAIT*/
			/*wait(NULL);*/
		}
		
	}
	getchar();
	print_map();

	/*Free all nodes and map*/
	free(node_list);
	free(map);
	exit(1);
}