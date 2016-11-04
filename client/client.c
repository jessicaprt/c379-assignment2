#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <poll.h>
 
int sock;

int open_connection(char * hostname, char * port){
    /*
     * Following function grabbed from the getaddrinfo man page.
     */
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(hostname, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "Unable to resolve hostname.\n");
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

	if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
		break;                  /* Success */

	close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
	    fprintf(stderr, "Could not connect\n");
	    exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    return sfd;
}

void endconnection(int signum) {
    printf("\nyou have left the chat\n");
    close(sock);
    exit(1);
}

int get_user_list(int sock) {
	size_t userlistsize = 1024;
    uint8_t userinfo_length;
    char userinfo_username[userlistsize];
	char connections[255];
	uint16_t numconnections;

   	printf("grabbing user infopooooooooo\n");

    uint16_t connected;
    printf("getting number of connections.\n");
	
	int s = recv(sock, &numconnections, sizeof(uint16_t), 0);
	if (s < 0) {
		printf("Error grabbing number of users");
		return -1;
	}

    connected = ntohs(numconnections); //should contain the number of users connected
    printf("%i \n", connected);		
    
    printf("number of users: %i\n", connected);

    int i = 0;
    while(i<connected) {
        read(sock, &userinfo_length, 1); // read length
        read(sock, userinfo_username, userinfo_length);

        printf("%s\n", userinfo_username);
        i++;
    }

    return 0;
}
 
int main (int argc, char * argv[]) { //input    : chat379 hostname portnumber username
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    struct hostent *server;
    const size_t buff_size = 1024;
    char buffer[255];
    char thisuserinfo[255];
    ssize_t read_size;
    int check;
    // struct pollfd sock_fds[200];
 
    signal(SIGTSTP, endconnection);
 
    if (argc != 4) {
        printf("Error, wrong number of arguments");
        exit(EXIT_FAILURE);
    }
 
    char *username = argv[3];
    size_t username_length = strlen(username);
 
    if (username_length > 255) {
        fprintf(stderr, "Username is too long. Maximum username length is 255 characters\n");
        exit(EXIT_FAILURE);
    }

    int sock = open_connection(argv[1], argv[2]);
 
    if (sock == -1) {
        printf("Error opening\n");
        exit(EXIT_FAILURE);
    }
 
    //Acknowledgement process
    buffer[buff_size];

    int s = 0;
    s = recv(sock, buffer, 2, 0);
	if (buffer[0] == (char) 0xCF && buffer[1] == (char) 0xA7){
		fprintf(stdout, "Connected to server.\n");
	} else {
		fprintf(stdout, "%hhx, %hhx\n", buffer[0], buffer[1]);
		fprintf(stderr, "Server does not speak protocol\n");
	}
	fprintf(stdout,"jj %s\n", buffer);

	/** get the user list from server **/
   	int getlist = get_user_list(sock);
   	printf("got list!\n");

    /*** send info to server ***/
    printf("sending user info to server\n");
	send(sock, &username_length, sizeof(uint8_t), 0);
	send(sock, username, username_length, 0);
	sprintf(thisuserinfo, "%i %s", (int)username_length, username);
	send(sock, thisuserinfo, sizeof(thisuserinfo), 0);
	printf("%s\n", thisuserinfo);
	printf("done sending to user\n");

    char joined[255];
    sprintf(joined, "%s has joined the chat!\n", username);
    int n = send(sock, (char*)&joined, strlen(joined), 0);
    if (n < 0) {
    	perror("ERROR sending username info");
    	exit(EXIT_FAILURE);
    }

    printf("%s has joined the chat!\n", username);
 
    /** begin sending message **/
    do {
        char user_message[255];
        printf("%s: ", username);
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
 
        sprintf(user_message, "%s: %s", username, buffer);
 
        // printf("sending\n");
        check = send(sock, user_message, strlen(user_message), 0);
        if (check < 0) perror("Error writing to socket");
        check = read(sock, buffer, strlen(buffer));
        if (check < 0) perror("Error reading from socket");
        printf("%s", buffer);
    } while (check > 0);
 
    close(sock);
    return 0;
}
