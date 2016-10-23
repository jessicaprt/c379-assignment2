#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main (int argc, char * argv[]) { //input	: chat379 hostname portnumber username
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	struct hostent *server;
	char buffer[255];
	ssize_t read_size;

	if (argc < 4) {
		printf("Error, too few arguments");
		exit(1);
	}

	char *username = argv[3];
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
    } else {
    	printf("c--connect success\n");
    }

    //Acknowledgement process
    char ackbuffer[2];
	unsigned char byte0, byte1;

    // printf("entering recv\n");
	while (read_size = recv(sock, (char*)&ackbuffer, 2, 0) > 0){
		printf("recieveing\n");
		printf ("%i\n", read_size);
		byte0 = *((unsigned char *)&ackbuffer + 0);
		printf("byte0 %x\n", byte0);
		byte1 = *((unsigned char *)&ackbuffer + 1);
		printf("byte1 %x\n", byte1);
	}
	
	// recv(sock, (char*)&ackbuffer, 2, 0);
	
	if (byte0 != 0xcf || byte1 != 0xa7) printf("byte failed\n");
	else printf("ack success");

	/**** get username info ****/
	int length = strlen(username);
	int name[2];
	name[0] = length; name[1] = atoi(username);
	write(sock, (char*)&name, sizeof(name) );

	/** check if username exists 
	 *  terminate connection if it exists
	**/

	//chat connection

    close(sock);
	return 0;
}