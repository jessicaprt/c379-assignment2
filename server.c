#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include "servcli.h"

int sock;

void add_username(node_t * head, char * username) {
	node_t * curr = head;
	while (curr->next != NULL) {
		curr = curr->next;
	}

	curr->next = malloc(sizeof(node_t));
	curr->next->user = username;
	curr->next->next = NULL;
}

void print_usernames(node_t * head) {
	node_t * curr = head;

	while(curr!=NULL) {
		printf("%s\n", curr->user);
		curr = curr->next;
	}
}

void endconnection(int signum) {
	printf("s: closing socket\n");
	close(sock);
	exit(1);
}

int main(int argc, char * argv[]) { //input		: server379 portnumber
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	char buffer[255];
	ssize_t read_size;
	struct pollfd sock_fds[200];
	int check, on = 1;
	int expired = false;
	int acceptsock = -1;

	//current number in sock_fds
	int nfds = 1;

	signal(SIGINT, endconnection);

	node_t * usernames = NULL;
	usernames = malloc(sizeof(node_t));
	usernames->user = "usernames: ";
	usernames->next = NULL;

	
	if (argc !=2) {
		perror("Error, incorrect number of arguments");
		exit(1);
	}

	/** send the following two bytes 0xCF 0xA7 **/
	int portnum = atoi(argv[1]);
	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		printf("Error opening");
		exit(1);
	}

	//allow socket to be reusable
	check = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if(check < 0) {
		perror("setsockopt failed");
		close(sock);
		exit(-1);
	}

	//set socket to be nonblocking
	check = ioctl(sock, FIONBIO, (char *)&on);
	if (check < 0) {
		perror("ioctl failed");
		close(sock);
		exit(-1);
	}

    bzero((char *) &srv_addr, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(portnum); //in network byte order

    //binding to socket
    check = bind(sock, (struct sockaddr*) &srv_addr, sizeof(srv_addr));
    if(check < 0) {
    	perror("Failed to bind socket");
    	close(sock);
    	exit(-1);
    }

    check = listen(sock, 20);
    if (check < 0) {
    	perror("Failed to listen to socket");
    	exit(-1);
    }

    socklen_t cli_size = sizeof(cli_addr);

    //initialize pollfd
    memset(sock_fds, 0, sizeof(sock_fds));

	/**** begin message passing here ****/

    sock_fds[0].fd = sock;
    sock_fds[0].events = POLLIN;

    int timeout = 10000; 

    do {

    	printf("waiting.. \n");

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

    	do {

		   	acceptsock = accept(sock, (struct sockaddr*) &cli_addr, &cli_size);

		    if(acceptsock == -1) {
		    	perror("Error accepting");
		    	exit(1);
		    }	

		    // Acknowledgement process
			unsigned char byte;
			char* ackbuffer[2];
			sprintf(ackbuffer, "\xCF\xA7\n");
			byte = *((unsigned char *)&ackbuffer + 0);
			byte = *((unsigned char *)&ackbuffer + 1);

			int n = send(acceptsock, (char*)&ackbuffer, 2, 0);
			if (n < 0) error("ERROR sending");

			//send the number of connected users
			int hex_num[1];
			sprintf(hex_num, "%x", nfds);
			n = send(acceptsock, (char*)&hex_num, 1, 0);

			if (n < 0) error("ERROR sending number of connections");

			/**** get client's info */
			char userinfo[255];
			char* saved_userinfo;
			printf("getting client's info\n");
			while (read_size = recv(acceptsock, &userinfo, 255, 0) > 0){
				printf("user info: %s\n", (char*)userinfo);
				add_username(usernames, (char*)userinfo);
				print_usernames(usernames);
				break;
			}
    		int i = 0;
	    	int nsize = nfds;

    		for (i=0; i < nsize; i++) {

    			printf("loop\n");	
	    		if(sock_fds[i].revents == 0) continue;

	    		if(sock_fds[i].revents != POLLIN) {
	    			printf("Error");
	    			expired = true;
	    			break;
	    		}

	    		// pollcheck = poll(sock_fds, nfds, 30000);
	    		if(sock_fds[i].fd == sock) {
	    			printf("socket is readable: \n");
	    			do {
	    				bzero(buffer, 256);
	    				check = read(acceptsock, buffer, 255);
	    				// printf("read check: %i\n", check);
	    				if (check<0) {
	    					printf("connection terminated by user\n");
	    					exit(1);
	    				}
	    				printf("user: %s", buffer);
	    				check = write(acceptsock, "message received", 16);
	    				// printf("write check: %i\n", check);
	    				if (check < 0) perror("error writing to socket");
					    nfds++;
	    			} while(acceptsock >0 | pollcheck > 0);
	    		}
	    	}    

    	} while(check > 0);



    	printf("poll did not timeout\n");

    	
    		
    } while(expired == false);

    printf("left while\n");
    int j = 0;
    for(j = 0; j< nfds; j++) {
    	if(sock_fds[j].fd >=0) close(sock_fds[j].fd);
    }

    close(sock);

	return 0;
}

/** to do:
 * - connection expires after 30 seconds of inactivity from client
 * - log files
 * - dealing with multiple connections
 * - how the client can terminate the connection (right now it's by ctrl+c)
 ***/