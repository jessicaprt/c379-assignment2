#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <poll.h>
#include <stdlib.h>

// Socket Handling
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"
#include "user.h"
#include "user_list.h"
#include "user_coms.h"


const size_t buff_size = 512;

void* user_handler_function(void* socket_ptr){
    uint8_t buffer[buff_size];
    uint8_t* buffer_ptr;
    uint8_t* end_buffer_ptr = buffer + buff_size;
    uint8_t name_length;
    uint16_t msg_length;

    int ps, s;

    int socket = *((int *) socket_ptr);
    free(socket_ptr);

    fprintf(log_stream, "Child Thread Socket ID: %d\n", socket);
    fflush(log_stream);
    s = send_user_list(socket);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to send user name list\n");
        fflush(log_stream);
        return NULL;
    }

    s = recv(socket, buffer, buff_size, 0);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to receive user name\n");
        fflush(log_stream);
        return NULL;
    }


    
    user_t* user;

    name_length = buffer[0];
    buffer_ptr = buffer + 1;

    if (is_name_used((char *) buffer_ptr, name_length)){
        close(socket);
        return NULL;
    }

    user = create_user((char *) buffer_ptr, name_length, socket);
    if (user == NULL){
        close(socket);
        fprintf(log_stream, "Unable to create user\n");
        fflush(log_stream);
        return NULL;
    }

    broadcast_user_join(user);
    append_user(user);

    ps = 0;
    struct pollfd user_fds[1];
    memset(user_fds, 0, sizeof(user_fds));
    user_fds[0].fd = socket;
    user_fds[0].events = POLLIN;
    const int user_timeout = 30 * 1000;

    msg_length = 0;

    while (is_running()){
        ps = poll(user_fds, 1, user_timeout);

        if (ps == 0){ // Poll timedout
            break;
        }

        if (user_fds[0].revents == POLLIN) {
            buffer_ptr = buffer;

            s = recv(socket, buffer, buff_size, 0);
            if (s< 0){
                break;
            }

            msg_length = ntohs((uint16_t) *buffer_ptr);
            buffer_ptr += 2;

            if (msg_length == 0){
                continue;
            }

            broadcast_msg(user, msg_length, (char *) buffer_ptr);
            buffer_ptr += msg_length;
        }
    }

    broadcast_user_quit(user);
    delete_user(user);
    return NULL;
}

void create_user_handler(int socket){
    int s;
    pthread_t thread;
    int* socket_ptr = malloc(sizeof(int));
    *socket_ptr = socket;
    s = pthread_create(&thread, NULL, user_handler_function, socket_ptr);
    if ( s != 0 ) {
        close(socket);
        fprintf(log_stream, "Unable create thread.");
        fflush(log_stream);
        return;
    }

}
