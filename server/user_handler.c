#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <poll.h>
#include <errno.h>
#include <stdlib.h>

// Socket Handling
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "server.h"
#include "user.h"
#include "user_list.h"
#include "user_coms.h"


const size_t buff_size = 1024;

void print_buffer(FILE* file_stream, uint8_t* buffer, size_t length){
    for (size_t i = 0; i < length; i++){
        fprintf(file_stream, "%hhx", buffer[i]);
    }
    fprintf(file_stream, "\n"); 
    fflush(file_stream);
}

void* user_handler_function(void* socket_ptr){
    int socket = *((int *) socket_ptr);
    free(socket_ptr);

    int ps, s, err;
    user_t* user;

    uint8_t buffer[buff_size];
    size_t offset;

    uint8_t name_length;
    uint16_t msg_length;

    fprintf(log_stream, "Child Thread Socket ID: %d\n", socket);
    fflush(log_stream);
    s = send_user_list(socket);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to send user name list\n");
        fflush(log_stream);
        return NULL;
    }

    memset(buffer, 0, buff_size);
    s = recv(socket, buffer, buff_size, 0);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to receive user name\n");
        fflush(log_stream);
        return NULL;
    }

    name_length = buffer[0];
    offset = 1;

    if (is_name_used((char *) buffer + offset, name_length)){
        close(socket);
        return NULL;
    }

    user = create_user((char *) buffer + offset, name_length, socket);
    if (user == NULL){
        close(socket);
        fprintf(log_stream, "Unable to create user\n");
        fflush(log_stream);
        return NULL;
    }

    broadcast_user_join(user);
    append_user(user);

    // Setup Polling
    ps = 0;
    struct pollfd user_fds[1];
    memset(user_fds, 0, sizeof(user_fds));
    user_fds[0].fd = socket;
    user_fds[0].events = POLLIN;
    const int user_timeout = 30 * 1000;

    offset = 0;
    msg_length = 0;

    while (is_running()){
        user_fds[0].revents = 0;
        ps = poll(user_fds, 1, user_timeout);
        err = errno;

        if (ps < 0) {
            fprintf(log_stream, "Error while polling: %s\n", strerror(err));
            fflush(log_stream);
            break;
        } else if (ps == 0){ // Poll timedout
            fprintf(log_stream, "Poll timedout\n");
            fflush(log_stream);
            break;
        } else if (user_fds[0].revents == POLLIN) {
            fflush(log_stream);
            s = recv(socket, buffer + offset, buff_size - offset, 0);
            if (s <= 0){
                break;
            }


            if(offset == 0 && s > 0) {
                memcpy(&msg_length, buffer + offset, sizeof(msg_length));
                msg_length = ntohs(msg_length);
                if (msg_length == 0){ // Keep Alive
                    continue;
                }
            }

            offset += s;

            fprintf(log_stream, "Offset: %d, MSGLength: %d\nBuffer: ", offset, msg_length);
            fflush(log_stream);
            print_buffer(log_stream, buffer, offset);

            if (offset - 2 < msg_length) {
                continue;
            } else if (offset - 2 >= msg_length) {
                broadcast_msg(user, msg_length, (char *) buffer + 2);

                memset(buffer, 0, buff_size);
                offset = 0;
                msg_length = 0;
            }
        } else {
            fprintf(log_stream, "Error while polling\n");
            fflush(log_stream);
            break;
        }

    }

    remove_user(user);
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
