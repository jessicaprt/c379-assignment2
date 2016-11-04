#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#include "server.h"
#include "user.h"
#include "user_list.h"
#include "user_coms.h"

const size_t buff_size = 512;

void* user_handler_function(void* passed_sd){
    uint8_t buffer[buff_size];
    uint8_t* buffer_ptr;
    uint8_t name_length;

    int socket = *((int *) passed_sd);
    int s = send_user_list(socket);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to send user name list\n");
        fflush(log_stream);
        return;
    }

    s = recv(socket, buffer, buff_size, 0);
    if (s < 0){
        close(socket);
        fprintf(log_stream, "Unable to receive user name\n");
        fflush(log_stream);
        return;
    }


    
    user_t* user;

    name_length = buffer[0];
    buffer_ptr = buffer + 1;

    if (is_name_used((char *) buffer_ptr, name_length)){
        close(socket);
        return;
    }

    user = create_user((char *) buffer_ptr, name_length, socket);
    if (user == NULL){
        close(socket);
        fprintf(log_stream, "Unable to create user\n");
        fflush(log_stream);
    }

    broadcast_user_join(user);
    append_user(user);

    while (is_running()){
        sleep(10);
    }

    broadcast_user_quit(user);
    delete_user(user);
}

void create_user_handler(int socket){
    int s;
    pthread_t thread;
    s = pthread_create(&thread, NULL, user_handler_function, &socket);
    if ( s != 0 ) {
        fprintf(log_stream, "Unable create thread.");
        fflush(log_stream);
        close(socket);
        return;
    }

}
