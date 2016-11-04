#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "server.h"
#include "user.h"
#include "user_list.h"
#include "user_coms.h"

int send_user_list(int socket){
    user_t* cuser;

    user_list_lock();

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

    user_list_unlock();

    return 0;
}

void broadcast_user_join(user_t* user){
    user_t* cuser;

    user_list_lock();

    cuser = user_list_head;

    uint8_t msg_type = 0x1;

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

    user_list_unlock();

    return 0;
}

void broadcast_user_quit(user_t* user){
    user_t* cuser;

    user_list_lock();

    cuser = user_list_head;

    uint8_t msg_type = 0x2;

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

    user_list_unlock();

    return 0;
}
