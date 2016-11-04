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
#include <pthread.h>
// #include "../server/get_user_list.c"
 
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

void keepalive(int signum) {
    uint16_t message_length = htons(0);
    send(sock, &message_length ,2 ,0);
    alarm(20);
}

void endconnection(int signum) {
    close(sock);
    exit(1);
}

int get_user_list(int sock) {
	size_t userlistsize = 1024;
    uint8_t userinfo_length;
    char userinfo_username[userlistsize];
	char connections[255];
	uint16_t numconnections;
	int check;

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

void * writeMessage(void * notused) {
	// size_t length = sizeof(uint16_t);
	char buffer[255];
    char user_message[255];
    int check;

	do {
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
 
        sprintf(user_message, "%s", buffer);
 
        // printf("sending\n");
        uint16_t message_length = htons(strlen(user_message));
        send(sock, &message_length ,2 ,0);
        if (check < 0) perror("Error writing to socket");
        check = send(sock, user_message, strlen(user_message), 0);
        if (check < 0) perror("Error writing to socket");
        // printf("%s", buffer);
    } while (check > 0);
}

void * getMessage(void * notused) {
	int check;
	uint8_t message_code;

	do {
		int bytesread = recv(sock, &message_code, sizeof(uint8_t), 0);
		if (bytesread == 0) {
			close(sock);
			exit(1);
		}
		if (message_code == 0x00) {
			uint16_t message_length;
			uint8_t user_length;
			char username[UINT8_MAX] = { 0 };
			char message[UINT16_MAX] = { 0 };

			recv(sock, &user_length, sizeof(uint8_t), 0);
			recv(sock, &username, user_length, 0);
			recv(sock, &message_length, sizeof(uint16_t), 0);
			recv(sock, &message, ntohs(message_length), 0);
			printf("%s: %s\n", username, message);
		}

		if (message_code == 0x01) {
			uint8_t user_length;
			char username[UINT8_MAX] = { 0 };
			recv(sock, &user_length, sizeof(uint8_t), 0);
			recv(sock, &username, user_length, 0);
			printf("%s has joined\n", username);
			// create_user(username, user_length, 0);
			// append_user(username);
		}

		if (message_code == 0x02) {
			uint8_t user_length;
			char username[UINT8_MAX] = { 0 };
			recv(sock, &user_length, sizeof(uint8_t), 0);
			recv(sock, &username, user_length, 0);
			printf("%s has left\n", username);
			// remove_user();
		}
		
	} while(1);
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

    pthread_t writeMessage_thread, getMessage_thread;
    int check1, check2;
    char * threadMessage2;

    struct sigaction sigterm;

    sigterm.sa_handler = keepalive;
 
    signal(SIGTSTP, endconnection);
    sigaction(SIGALRM, &sigterm, NULL);
    alarm(20);

 
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

    sock = open_connection(argv[1], argv[2]);
 
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

	/** get the user list from server **/
   	int getlist = get_user_list(sock);
   	printf("got list!\n");

    /*** send info to server ***/
    printf("sending user info to server\n");
	send(sock, &username_length, sizeof(uint8_t), 0);
	send(sock, username, username_length, 0);
	sprintf(thisuserinfo, "%i %s", (int)username_length, username);
	printf("%s\n", thisuserinfo);
	printf("done sending to user\n");

    char joined[255];
 
    // /** begin sending message **/
    check1 = pthread_create(&getMessage_thread, NULL, getMessage, NULL);
    if (check1) {
    	perror("error getting message (thread error)");
    	exit(EXIT_FAILURE);
    }


    check2 = pthread_create(&writeMessage_thread, NULL, writeMessage, NULL);
    if (check2) {
    	perror("error writing message (thread error)");
    }
    // writeMessage(NULL);
    pthread_join(getMessage_thread, NULL);
    pthread_join(writeMessage_thread, NULL);
 
    return 0;
}
