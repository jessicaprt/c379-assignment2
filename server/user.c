#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

#include "server.h"
#include "user.h"
#include "user_list.h"

void broadcast_user_join(user_t* user){

}

void broadcast_user_quit(user_t* user){

}

void user_handler_function(void* passed_sd){
    int s = send_user_list(passed_sd);
    
    user_t* user;

    broadcast_user_join(user);

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
