#include <stdio.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

// GLOBALS

FILE * log_stream = NULL;

// END GLOBALS

int main(int argc, char * argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int logs = daemonize();
    // Can not use stdout, stdin, stderr anymore...
    
    int signals = open_signal_fd();
    int sock = open_main_socket(argv[1]);

    const int n_mp_fds = 2;
    struct pollfd mp_fds[n_mp_fds];
    memset(mp_fds, 0 , sizeof(mp_fds));
    mp_fds[0].fd = sock;
    mp_fds[0].events = POLLIN;
    mp_fds[1].fd = signals;
    mp_fds[1].events = POLLIN;
    const int mp_timeout = -1;
    int poll_status;

    // event loop
    start_running();
    while(is_running()) {

        fprintf(log_stream, "Polling for new connection or signal\n");
        poll_status = poll(mp_fds, n_mp_fds, mp_timeout);

        if(mp_fds[1].revents == POLLIN) {
            handle_signal(signals);
            continue;
        }

        if(mp_fds[0].revents == POLLIN) {
            accept_new_connections(sock);
        }
        
    }

    close(sock);
    close(signals);
    fprintf(log_stream, "Terminating...\n");
    fflush(log_stream);
    close(logs);
}
