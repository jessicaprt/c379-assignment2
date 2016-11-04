#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "server.h"

int open_main_socket(char* port){
    /* Function from getnameinfo man page */
    /*  http://www.ibm.com/support/knowledgecenter/ssw_ibm_i_71/rzab6/poll.htm */
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, rc, on;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    
    on = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;/* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(log_stream, "Unable to lookup local connection information");
        fflush(log_stream);
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;

        /* Allow socket descriptor to be reuseable */
        rc = setsockopt(sfd, SOL_SOCKET,  SO_REUSEADDR,
                (char *)&on, sizeof(on));
        if (rc < 0)
        {
            close(sfd);
            continue;
        }

        /* Set socket to be nonblocking */
        rc = ioctl(sfd, FIONBIO, (char *)&on);
        if (rc < 0)
        {
            close(sfd);
            continue;
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(log_stream, "Could not bind to a port\n");
        fflush(log_stream);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */

    rc = listen(sfd, 32);
    if (rc < 0)
    {
        fprintf(log_stream, "Could not set to passive port\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    rc = fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
    if (rc == -1){
        fprintf(log_stream, "Unable to set file descriptor to nonblocking\n");
        fflush(log_stream);
        exit(EXIT_FAILURE);
    }

    return sfd;
}

int open_signal_fd(){
    sigset_t mask;
    int sfd;
    struct signalfd_siginfo fdsi;
    ssize_t size;
    int d;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    /* Block signals so that they aren't handled
     *        according to their default dispositions */

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1){
    }

    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1) {
        fprintf(log_stream, "Unable to open signal handler file descriptor\n");
        fflush(log_stream);
        exit(EXIT_FAILURE);
    }

    d = fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
    if (d == -1){
        fprintf(log_stream, "Unable to set file descriptor to nonblocking\n");
        fflush(log_stream);
        exit(EXIT_FAILURE);
    }

    return sfd;
}
