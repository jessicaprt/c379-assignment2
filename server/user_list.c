#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "server.h"
#include "user.h"
#include "user_list.h"

static pthread_mutex_t user_list_check_mutex;
static pthread_mutex_t user_list_mutex;
static pthread_once_t user_list_once = PTHREAD_ONCE_INIT;
uint64_t user_list_reader_count = 0;

user_t* user_list_head = NULL;
user_t* user_list_tail = NULL;
uint16_t user_list_length = 0;

void user_list_init(){
    pthread_mutex_init(&user_list_check_mutex, NULL);
    pthread_mutex_init(&user_list_mutex, NULL);
}

int append_user(user_t* user){

    if (user->n != NULL && user->p != NULL){
        return -1; // User already in list.    
    }   

    user_list_write_lock();

    if(user_list_head == NULL){
        user_list_head = user;
    }   
    if(user_list_tail != NULL){
        user_list_tail->n = user;
    }   
    user->p = user_list_tail;
    user_list_tail = user;

    user_list_length += 1;

    user_list_write_unlock();
    
    return 0;
}


user_t* create_user(char* name, uint8_t name_length, int socket){
    fprintf(log_stream, "Allocating User Object\n");
    fflush(log_stream);

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

    user->lock = malloc(sizeof(pthread_mutex_t));
    if(user->lock == NULL){
        return NULL;
    }
    memset(user->lock, 0, sizeof(pthread_mutex_t));

    if( pthread_mutex_init(user->lock, NULL) != 0){
        free(user->name);
        free(user);

        return NULL;
    }

    user->n = NULL;
    user->p = NULL;
    
    fprintf(log_stream, "Successfully Allocated User Object\n");
    fflush(log_stream);

    return user;
}

int remove_user(user_t* user){
    if (user->n == NULL && user->p == NULL){
        return -1; // user in list
    }
    user_list_write_lock();

    if(user_list_tail != NULL){
        user_list_tail->n = user->n;
    }
    if(user_list_head != NULL){
        user_list_head->p = user->p;
    }

    user_list_length -= 1;
    user_list_write_unlock();

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

int64_t user_list_read_lock(){
    pthread_once(&user_list_once, user_list_init);

    pthread_mutex_lock(&user_list_check_mutex);
    pthread_mutex_lock(&user_list_mutex);
    user_list_reader_count += 1;
    
    pthread_mutex_unlock(&user_list_mutex);
    pthread_mutex_unlock(&user_list_check_mutex);

    return user_list_reader_count;
}

int64_t user_list_read_unlock(){
    pthread_mutex_lock(&user_list_check_mutex);
    if(user_list_reader_count > 0){
        user_list_reader_count -= 1;
    }
    pthread_mutex_unlock(&user_list_check_mutex);

    return user_list_reader_count;
}

int user_list_write_lock(){
    int s;
    pthread_once(&user_list_once, user_list_init);

    pthread_mutex_lock(&user_list_check_mutex);
    pthread_mutex_lock(&user_list_mutex);
    pthread_mutex_unlock(&user_list_check_mutex);

    while(user_list_reader_count > 0);

    return s;
}

int user_list_write_unlock(){
    pthread_mutex_unlock(&user_list_mutex);
    return 0;
}

user_t* find_user_by_name(char* name, uint8_t length){
    user_t* cuser;

    user_list_read_lock();

    char tmp_name[length + 1];
    strncpy(tmp_name, name, length);
    tmp_name[length] = '\0';
    fprintf(log_stream, "Checking to see if user %s exists.\n", tmp_name);
    fflush(log_stream);


    cuser = user_list_head;
    while (cuser != NULL){
        if(length == cuser->name_length){
            if(strncmp(name, cuser->name, length) == 0){ 
                user_list_read_unlock();
                fprintf(log_stream, "User exists\n");
                fflush(log_stream);
                return cuser;
            }
        }

        cuser = cuser->n;
    }   

    user_list_read_unlock();

    fprintf(log_stream, "User does not exist\n");
    fflush(log_stream);
    return NULL;
}

int is_name_used(char* name, uint8_t length){
    if (find_user_by_name(name, length) == NULL){
        return 0;
    }

    return 1;
}
