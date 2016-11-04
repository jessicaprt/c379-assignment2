#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "server.h"
#include "user.h"
#include "user_list.h"
#include "user_coms.h"

int send_user_list(int socket){
    user_t* cuser;

    user_list_read_lock();

    cuser = user_list_head;

    uint16_t nusl = htons(user_list_length);
    int s = 0;

    s = send(socket, &nusl, sizeof(uint16_t), 0);
    if (s < 0){
        return -1;
    };

    while (cuser != NULL){
        s = send(socket, &(cuser->name_length), sizeof(uint8_t), 0);
        if (s < 0){
            return -1;
        };

        s = send(socket, cuser->name, cuser->name_length, 0);
        if (s < 0){
            return -1;
        };


        cuser = cuser->n;
    }

    user_list_read_unlock();

    return 0;
}

void broadcast_user_join(user_t* user){

    int s = 0;

    char tmp_name[user->name_length + 1];
    strncpy(tmp_name, user->name, user->name_length);
    tmp_name[user->name_length] = '\0';
    fprintf(log_stream, "User %s quit.\n", tmp_name);
    fflush(log_stream);

    user_t* cuser;

    user_list_read_lock();

    cuser = user_list_head;

    uint8_t msg_type = 0x01;

    while (cuser != NULL){
        pthread_mutex_lock(cuser->lock);

        s = send(cuser->socket, &(user->name_length), sizeof(uint8_t), 0);
        if (s < 0){
            continue;
        };

        s = send(cuser->socket, user->name, user->name_length, 0);
        if (s < 0){
            continue;
        };

        pthread_mutex_unlock(cuser->lock);


        cuser = cuser->n;
    }

    user_list_read_unlock();
}

void broadcast_user_quit(user_t* user){

    int s = 0;

    char tmp_name[user->name_length + 1];
    strncpy(tmp_name, user->name, user->name_length);
    tmp_name[user->name_length] = '\0';
    fprintf(log_stream, "User %s quit.\n", tmp_name);
    fflush(log_stream);

    user_t* cuser;

    user_list_read_lock();

    cuser = user_list_head;

    uint8_t msg_type = 0x02;

    while (cuser != NULL){
        pthread_mutex_lock(cuser->lock);

        s = send(cuser->socket, &(user->name_length), sizeof(uint8_t), 0);
        if (s < 0){
            continue;
        };

        s = send(cuser->socket, user->name, user->name_length, 0);
        if (s < 0){
            continue;
        };

        pthread_mutex_unlock(cuser->lock);


        cuser = cuser->n;
    }

    user_list_read_unlock();
}

void broadcast_msg(user_t* user, uint16_t msg_length, char* msg){
    int s = 0;

    char tmp_name[user->name_length + 1];
    char tmp_msg[msg_length + 1];
    strncpy(tmp_name, user->name, user->name_length);
    strncpy(tmp_msg, msg, msg_length);
    tmp_name[user->name_length] = '\0';
    tmp_msg[msg_length] = '\0';
    fprintf(log_stream, "Msg: %s: %s\n", tmp_name, tmp_msg);
    fflush(log_stream);


    user_t* cuser;

    user_list_read_lock();

    cuser = user_list_head;

    uint16_t nusl = htons(msg_length);

    uint8_t msg_type = 0x00;

    while (cuser != NULL){
        pthread_mutex_lock(cuser->lock);

        s = send(cuser->socket, &(user->name_length), sizeof(uint8_t), 0);
        if (s < 0){
            continue;
        };

        s = send(cuser->socket, user->name, user->name_length, 0);
        if (s < 0){
            continue;
        };

        s = send(cuser->socket, &nusl, sizeof(uint16_t), 0);
        if (s < 0){
            continue;
        };

        s = send(cuser->socket, msg, msg_length, 0);
        if (s < 0){
            continue;
        };
        pthread_mutex_unlock(cuser->lock);

        cuser = cuser->n;
    }

    user_list_read_unlock();
}
