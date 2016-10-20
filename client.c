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

	if (argc < 4) {
		fprintf("Error, too few arguments");
		exit(1);
	}

	char *username = argv[3];
	int portnumber = atoi(argv[2]);
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		fprintf("Error opening");
		exit(1);
	}
	server = gethostname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &srv_addr, sizeof(srv_addr));
    bcopy((char *)server->h_addr, (char *)&srv_addr.sin_addr.s_addr, server->h_length);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(portno);

    //establish connection to server
    if(connect(sock, &srv_addr, sizeof(srv_addr)) == -1) {
    	perror("Error connnecting");
    }

    //Acknowledgement process
    int number = 0;
	recv(sock, (char*)&number, sizeof(number), 0);
	if(number[0] != 0xCF || number[1] || 0xA7){
		perror("Acknowledgement Failed");
	}

	/**** get username info ****/
	int length = strlen(username);
	int name[2];
	name[0] = length; name[1] = atoi(username);
	send(sock, (char*)&name, sizeof(name), 0);

	/** check if username exists 
	 *  terminate connection if it exists
	**/

	return 0;
}