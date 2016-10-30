#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "servcli.h"

int sock;

void endconnection(int signum) {
	printf("c: closing socket\n");
	close(sock);
	exit(1);
}

int main (int argc, char * argv[]) { //input	: chat379 hostname portnumber username
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	struct hostent *server;
	char buffer[255];
	ssize_t read_size;
	int check;
	struct pollfd sock_fds[200];

	signal(SIGINT, endconnection);

	if (argc != 4) {
		printf("Error, wrong number of arguments");
		exit(1);
	}

	char *username = argv[3];
	unsigned int username_length = strlen(username);
	int portnumber = atoi(argv[2]);
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		printf("Error opening\n");
		exit(1);
	}

	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &srv_addr, sizeof(srv_addr));
    bcopy((char *)server->h_addr, (char *)&srv_addr.sin_addr.s_addr, server->h_length);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(portnumber);

    //establish connection to server
    if(connect(sock,(struct sockaddr *) &srv_addr, sizeof(srv_addr)) == -1) {
    	perror("Error connnecting\n");
	}

	//Acknowledgement process
    char ackbuffer[2];
	unsigned char byte0, byte1;

    // printf("entering recv\n");
	while (read_size = recv(sock, (char*)&ackbuffer, 2, 0) > 0){
		// printf("recieveing acknowledgement\n");
		// printf ("%i\n", read_size);
		byte0 = *((unsigned char *)&ackbuffer);
		// printf("byte0 %x\n", byte0);
		byte1 = *((unsigned char *)&ackbuffer + 1);
		// printf("byte1 %x\n", byte1);
		break;
	}
	
	printf("done receiving ack\n");
	if (byte0 != 0xcf || byte1 != 0xa7) printf("byte failed\n");
	// else printf("ack success\n");


	/**** get number of connections from server; */
	char connections[2];
	unsigned char connected_str;
	unsigned int connected;
	while (read_size = recv(sock, (char*)&connections, 2, 0) > 0){
		// printf("recieveing number of connections\n");
		// connected_str = *((unsigned char *)&connections);
		connected = atoi((char *)&connections);
		printf("number connected: %i\n", connected);
		break;
	}

	/**** send username info to server****/
	char sendusername[255];
	sprintf(sendusername, "%x %s", username_length, username);
	// printf("buff: %s", sendusername);
	// printf("size: %i\n", strlen(sendusername));
	int n = send(sock, (char*)&sendusername, strlen(sendusername), 0);
	if (n < 0) error("ERROR sending number of connections");
	printf("ack done\n");
	// send(sock, (char*)&sendusername, strlen(sendusername), 0);

	// check if username exists terminate connection if it exists

	sock_fds[0].fd = sock;
    sock_fds[0].events = POLLOUT;

    int timeout = 10000;
    int nfds = 1;

	/** begin message passing **/
	while(true) {
		printf("looooooping\n");
		int pollcheck = poll(sock_fds, nfds, timeout);
    	printf("check: %i\n", pollcheck);

	   	if (pollcheck < 0) {
    		perror("poll failed");
    		break;
    	}

    	if (pollcheck == 0) {
    		printf("timeout\n");
    		break;
    	}

    	// do {
    		printf("connecting..\n");
		 	printf("%s: ", username);
		 	bzero(buffer, 256);
		 	fgets(buffer, 255, stdin);
		 	printf("sending\n");
		 	check = send(sock, buffer, strlen(buffer), 0);
		 	if (check < 0) perror("Error writing to socket");
		 	// check = read(sock, buffer, strlen(buffer));
		 	// if (check < 0) perror("Error reading from socket");
		 	// printf("serv: %s\n", buffer);
		 // } while(check > 0 | pollcheck >0);
	 	
	 }

	//chat connection

    close(sock);
	return 0;
}