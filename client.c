#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


int main () {
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;

	if (argc < 4) {
		fprintf("Error, too few arguments");
		exit(1);
	}
	
	return 0;
}