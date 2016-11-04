#ifndef __USER_H__
#define __USER_H__

typedef struct user{
    uint8_t name_length;
    char* name;
    int socket;
    pthread_mutex_t* lock;

    struct user* n;
    struct user* p;
} user_t;

void create_user_handler(int socket);

#endif
