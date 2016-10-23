#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>

/***
struct sockaddr_in
	short sin_family			: address family (AF_INET)
	u_short sin_port			: portnumber in network byte order
	struct in_addr  sin_addr	: IP address
	char sin_zero[8]			: 0 

struct in_addr
	unsigned long s_addr 		: IP address of host (INADDR_ANY)

****/

typedef int bool;
#define true 1
#define false 0

int numconnections = 0;

int main(int argc, char * argv[]) { //input		: server379 portnumber
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	char buffer[255];
	ssize_t read_size;
	
	// if (argc < 1) {
	// 	printf("Error, too few arguments");
	// 	exit(1);
	// }

	/** send the following two bytes 0xCF 0xA7 **/
	int portnum = atoi(argv[1]);
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		printf("Error opening");
		exit(1);
	}

	//set all values in a buffer to zero, initialize srv_addr to zero
    bzero((char *) &srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(portnum); //in network byte order

    //binding to socket
    if(bind(sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) == -1) {
    	perror("Failed to bind socket");
    }

    listen(sock, 20);

    socklen_t cli_size = sizeof(cli_addr);

    int acceptsock = accept(sock, (struct sockaddr*) &cli_addr, &cli_size);
    numconnections = numconnections + 1;

    if(acceptsock == -1) {
    	perror("Error accepting");
    	exit(1);
    } else {
    	// printf("s-- connect success\n");
    }

	// Acknowledgement process
	// int ACK[2];
	// ACK[0] = 0xCF; ACK[1] = 0xA7;
	unsigned char byte;
	// printf("sending ack\n");
	char* ackbuffer[2];
	sprintf(ackbuffer, "\xCF\xA7\n");
	byte = *((unsigned char *)&ackbuffer + 0);
	// printf("server buffer %x \n", byte);
	byte = *((unsigned char *)&ackbuffer + 1);
	// printf("server buffer %x \n", byte);

	int n = send(acceptsock, (char*)&ackbuffer, 2, 0);
	if (n < 0) error("ERROR sending");
	// else printf("success\n");

	//send the number of connected users
	int hex_num[1];
	sprintf(hex_num, "%x", numconnections);
	n = send(acceptsock, (char*)&hex_num, 1, 0);

	if (n < 0) error("ERROR sending number of connections");
	// else printf("sent number of connections\n");

	/***** send username info : length string (use linked list) */

	/**** get client's info */
	char userinfo[255];
	char* saved_userinfo;
	printf("getting client's info\n");
	while (read_size = recv(acceptsock, &userinfo, 255, 0) > 0){
		printf("got inside loop\n");
		printf("user info: %s\n", (char*)userinfo);
		break;
	}

	/***** check if username sent by client already exists */
	numconnections = numconnections - 1;
	close(sock);
	close(acceptsock);
	return 0;
}

        // if( send(sock , message , strlen(message) , 0) < 0)
        // {
        //     puts("Send failed");
        //     return 1;
        // }