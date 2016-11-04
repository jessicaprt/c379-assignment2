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
    printf("c: closing socket\n");
    close(sock);
    exit(1);
}
 
int main (int argc, char * argv[]) { //input    : chat379 hostname portnumber username
    struct sockaddr_in srv_addr;
    struct sockaddr_in cli_addr;
    struct hostent *server;
    char buffer[255];
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
        exit(1);
    }
 
    //Acknowledgement process
    const size_t buff_size = 1024;
    buffer[buff_size];
    // unsigned char byte0, byte1;
 
    // while (read_size = recv(sock, buffer, 2, 0) > 0){
    //     //loops while waiting for bytes to be received
    //     byte0 = *((unsigned char *)&buffer);
    //     byte1 = *((unsigned char *)&buffer + 1);
    //     break;
    // }

    // if (byte0 != (char) 0xcf || byte1 != (char) 0xa7) printf("byte failed\n");
    int s = 0;
    s = recv(sock, buffer, buff_size, 0);
	if (buffer[0] == (char) 0xCF && buffer[1] == (char) 0xA7){
		fprintf(stdout, "Connected to server.\n");
		printf("sengding user info to server\n");
		send(sock, &username_length, sizeof(uint8_t), 0);
		send(sock, username, username_length, 0);
		printf("done sending to user\n");
	} else {
		fprintf(stdout, "%hhx, %hhx\n", buffer[0], buffer[1]);
		fprintf(stderr, "Server does not speak protocol\n");
	}
 
    /**** get number of connections from server; */
    char connections[2];
    unsigned char connected_str;
    unsigned int connected;
    printf("getting number of connections..\n");

    s = recv(sock, (char*)&connections, 2, 0);
    connected = atoi((char *)&connections); //should contain the number of users connected
    printf("number of users: %i\n", connected);
    printf("Users connected: ");

    char userinfolist[255];
    int i = 0;
    while(i<connected) {
        read(sock, userinfolist, sizeof(buffer));
        printf("%s\n", userinfolist);
    }
    
    char joined[255];
    sprintf(joined, "%s has joined the chat!\n", username);
    n = send(sock, (char*)&joined, strlen(joined), 0);
    if (n < 0) error("ERROR sending username info");
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
