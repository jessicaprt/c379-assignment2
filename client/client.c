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
    struct pollfd sock_fds[200];
 
    signal(SIGTSTP, endconnection);
 
    if (argc != 4) {
        printf("Error, wrong number of arguments");
        exit(1);
    }
 
    char *username = argv[3];
    unsigned int username_length = strlen(username);
 
    if (username_length > 255) {
        fprintf(stderr, "Username is too long. Maximum username length is 255 characters\n");
        exit(EXIT_FAILURE);
    }
 
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
 
    while (read_size = recv(sock, (char*)&ackbuffer, 2, 0) > 0){
        //loops while waiting for bytes to be received
        byte0 = *((unsigned char *)&ackbuffer);
        byte1 = *((unsigned char *)&ackbuffer + 1);
        break;
    }
   
    printf("done receiving ack\n");
    if (byte0 != 0xcf || byte1 != 0xa7) printf("byte failed\n");
 
 
    /**** get number of connections from server; */
    char connections[2];
    unsigned char connected_str;
    unsigned int connected;
    while (read_size = recv(sock, (char*)&connections, 2, 0) > 0){
        //loops while waiting for bytes to be received
        connected = atoi((char *)&connections); //should contain the number of users connected
        printf("number of users: %i\n", connected);
        printf("Users connected: ");
        break;
    }
    char userinfolist[255];
    int i = 0;
    while(i<connected) {
        read(sock, userinfolist, sizeof(buffer));
        printf("%s\n", userinfolist);
    }
 
    /**** send username info to server****/
    char sendusername[255];
    sprintf(sendusername, "%x %s", username_length, username);
    printf("buff: %s", sendusername);
    printf("size: %i\n", strlen(sendusername));
    int n = send(sock, (char*)&sendusername, strlen(sendusername), 0);
    if (n < 0) error("ERROR sending username info");
 
    char joined[255];
    sprintf(joined, "%s has joined the chat!\n", username);
    n = send(sock, (char*)&joined, strlen(joined), 0);
    if (n < 0) error("ERROR sending username info");
    printf("%s has joined the chat!\n", username);
 
    /** begin sending message **/
    while (check > 0) {
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
    }
 
    close(sock);
    return 0;
}
