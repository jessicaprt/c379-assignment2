#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "user.h"
#include "user_list.h"

static pthread_mutex_t user_list_mutex;
static pthread_once_t user_list_once = PTHREAD_ONCE_INIT;

user_t* user_list_head = NULL;
user_t* user_list_tail = NULL;
uint16_t user_list_length = 0;

void user_list_init(){
    pthread_mutex_init(&user_list_mutex, NULL);
}

int append_user(user_t* user){

    if (user->n != NULL && user->p != NULL){
        return -1; // User already in list.    
    }   

    user_list_lock();

    if(user_list_head == NULL){
        user_list_head = user;
    }   
    if(user_list_tail != NULL){
        user_list_tail->n = user;
    }   
    user->p = user_list_tail;
    user_list_tail = user;

    user_list_length += 1;

    user_list_unlock();
    
    return 0;
}


user_t* create_user(char* name, uint8_t name_length, int socket){
    
    user_t* user = malloc(sizeof(user_t));
    if(user == NULL){
        return NULL;
    }
    memset(user, 0, sizeof(user_t));

    user->name = malloc(name_length);
    if(user->name == NULL){
        free(user);
        return NULL;
    }
    memset(user->name, 0, name_length);
    strncpy(user->name, name, name_length);

    user->name_length = name_length;

    user->socket = socket;

    if(pthread_mutex_init(user->lock, NULL) != 0){
        free(user->name);
        free(user);
        return NULL;
    }

    user->n = NULL;
    user->p = NULL;
    
    return user;

}

int remove_user(user_t* user){
    if (user->n == NULL && user->p == NULL){
        return -1; // user in list
    }
    user_list_lock();

    if(user_list_tail != NULL){
        user_list_tail->n = user->n;
    }
    if(user_list_head != NULL){
        user_list_head->p = user->p;
    }

    user_list_length -= 1;
    user_list_unlock();

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

int user_list_lock(){
    pthread_once(&user_list_once, user_list_init);
    return pthread_mutex_lock(&user_list_mutex);
}

int user_list_unlock(){
    pthread_once(&user_list_once, user_list_init);
    return pthread_mutex_unlock(&user_list_mutex);
}

int is_name_used(char* name, uint8_t length){
    user_t* cuser;

    user_list_lock();

    cuser = user_list_head;
    while (cuser != NULL){
        if(length == cuser->name_length){
            if(strncmp(name, cuser->name, length) == 0){ 
                user_list_unlock();
                return 1;
            }
        }

        cuser = cuser->n;
    }   

    user_list_unlock();

    return 0;
}

