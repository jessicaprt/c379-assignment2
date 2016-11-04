#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <signal.h>

#define true 1
#define false 0

int numconnections;

typedef struct node {
	char * user;
	struct node * next;
} node_t;