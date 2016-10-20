#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

//INITIAL HANDSHAKE

/***
struct sockaddr_in
	short sin_family			: address family (AF_INET)
	u_short sin_port			: portnumber in network byte order
	struct in_addr  sin_addr	: IP address
	char sin_zero[8]			: 0 

struct in_addr
	unsigned long s_addr 		: IP address of host (INADDR_ANY)

****/


int main(int argc, char * argv[]) { //input server379 portnumber
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	int numconnections = 5; // HOW TO GET NUMBER OF USERS CONNECTED?
	
	if (argc < 2) {
		fprintf("Error, too few arguments");
		exit(1);
	}

	/** send the following two bytes 0xCF 0xA7 **/
	int portnum = argv[1];
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		fprintf("Error opening");
		exit(1);
	}

	//set all values in a buffer to zero, initialize srv_addr to zero
    bzero((char *) &serv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(portnumber); //in network byte order

    //binding to socket
    if(bind(sock, (struck sockaddr*) &srv_addr, sizeof(srv_addr)) == -1) {
    	perror("Failed to bind socket");
    }

    listen(sock, numconnections);

    int acceptsock = accept(sock, (struct sockaddr*) &cli_addr, sizeof(cli_addr));
    if(acceptsock == -1) {
    	perror("Error accepting");
    	exit(1);
    }

	// Acknowledgement process
	int ACK[2];
	ACK[0] = 0xCF; ACK[1] = 0xA7;
	send(sock, (char*)&ACK, sizeof(ACK), 0);

	unsigned int num_connected = htons(numconnections);
	send(sock, (char*)&num_connected, sizeof(num_connected), 0);

	/***** send username info : length string */


	return 0;
}