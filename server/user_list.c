#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "user_list.h"

user_t * user_list_head = NULL;
user_t * user_list_tail = NULL;
uint16_t user_list_length = 0;

static pthread_mutex_t user_list_mutex;
static pthread_once_t user_list_once = PTHREAD_ONCE_INIT;

void user_list_init(){
    pthread_mutex_init(&user_list_mutex, NULL);
}

int append_user(user_t* user){
    pthread_once(&user_list_once, user_list_init);

    if (user->n != NULL && user->p != NULL){
        return -1; // User already in list.    
    }   

    pthread_mutex_lock(&user_list_mutex);

    if(user_list_head == NULL){
        user_list_head = user;
    }   
    if(user_list_tail != NULL){
        user_list_tail->n = user;
    }   
    user->p = user_list_tail;
    user_list_tail = user;

    user_list_length += 1;

    pthread_mutex_unlock(&user_list_mutex);
}


user_t* create_user(char* name, uint8_t name_length, int socket){
    user_t* user = malloc(sizeof(user_t));
    memset(user, 0, sizeof(user_t));

    user->name = name;
    user->name_length = name_length;
    user->socket = socket;
    pthread_mutex_init(user->lock, NULL);

    user->n = NULL;
    user->p = NULL;
    
    return user;

}

int remove_user(user_t* user){
    if (user->n == NULL && user->p == NULL){
        return -1; // user in list
    }
    pthread_mutex_lock(&user_list_mutex);

    if(user_list_tail != NULL){
        user_list_tail->n = user->n;
    }
    if(user_list_head != NULL){
        user_list_head->p = user->p;
    }

    user_list_length -= 1;

    pthread_mutex_unlock(&user_list_mutex);
    return 0;
}

int delete_user(user_t* user){
    pthread_mutex_lock(user->lock);
    remove_user(user);

    close(user->socket);
    free(user->name);
    pthread_mutex_unlock(user->lock);
    pthread_mutex_destroy(user->lock);
    free(user);

    return 0;
}

int send_user_list(int socket){
    user_t* cuser;

    pthread_mutex_lock(&user_list_mutex);
    cuser = user_list_head;
    
    uint16_t nusl = htons(user_list_length);
    int s = 0;

    s = send(socket, &nusl, sizeof(uint16_t), 0);
    if (s < 0){
        return -1;
    };

    while (cuser != NULL){
        s = send(socket, &(user->namelength), sizeof(uint8_t), 0);
        if (s < 0){
            return -1;
        };

        s = send(socket, user->name, user->namelength, 0);

        cuser = cuser->n;
    }
    pthread_mutex_unlock(&user_list_mutex);
    return 0;
}
