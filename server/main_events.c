#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/signalfd.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

#include "server.h"
#include "user.h"

// Event Loop functions
void handle_signal(int sigfd){
    struct signalfd_siginfo fdsi;
    int status = 0;

    status = read(sigfd, &fdsi, sizeof(struct signalfd_siginfo));
    if (status != sizeof(struct signalfd_siginfo)){
        fprintf(log_stream, "Signal file malformed\n");
        fflush(log_stream);
        stop_running();
    }

    if (fdsi.ssi_signo == SIGTERM) {
        fprintf(log_stream, "SIGTERM Received starting termination.\n");
        fflush(log_stream);
        stop_running();
    } else if (fdsi.ssi_signo == SIGINT) {
        fprintf(log_stream, "SIGINT Received. Server interupted. Resuming.\n");
        fflush(log_stream);
    }


}

void initiate_handshake(int socket){
    uint16_t network_magic = htons(0xCFA7);
    int s;
    s = send(socket, &network_magic, 2, MSG_DONTWAIT);
    if (s<0) {
        close(socket);
        fprintf(log_stream, "Unable to send initial handshake message.\n");
        fflush(log_stream);
        return;
    }
    create_user_handler(socket);
}

void accept_new_connections(int listensd){
    int tmp_sd = -1;
    do { 
        tmp_sd = accept(listensd, NULL, NULL); 
        if (tmp_sd < 0){
            if (errno != EWOULDBLOCK){
                fprintf(log_stream, "Accept Failed\n");
                fflush(log_stream);
                stop_running();
            }
            break;
        }
        fprintf(log_stream, "New Connection Accepted Start Handshake\n");
        fflush(log_stream);

        initiate_handshake(tmp_sd);

    } while (tmp_sd != -1);
}
