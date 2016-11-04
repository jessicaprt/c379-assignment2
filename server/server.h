#ifndef __SERVER_H__
#define __SERVER_H__

extern FILE * log_stream;

int daemonize();

int open_main_socket(char* port);
int open_signal_fd();

void start_running();
int is_running();
void stop_running();

void handle_signal(int sigfd);
void initiate_handshake(int socket);
void accept_new_connections(int listensd);


#endif
