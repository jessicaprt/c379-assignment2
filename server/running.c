#include <stdio.h>
#include <pthread.h>

#include "server.h"

// Event loop running variable
static pthread_mutex_t running_mutex;
static pthread_once_t running_once = PTHREAD_ONCE_INIT;
int running;

void run_init(){
    pthread_mutex_init(&running_mutex, NULL);
}

void start_running(){
    pthread_once(&running_once, run_init);    
    pthread_mutex_lock(&running_mutex);
    running = 1;
    pthread_mutex_unlock(&running_mutex);
}

int is_running(){
    int r = -1;
    pthread_mutex_lock(&running_mutex);
    r = running;
    pthread_mutex_unlock(&running_mutex);
    return r;
}

void stop_running(){
    pthread_mutex_lock(&running_mutex);
    running = 0;
    pthread_mutex_unlock(&running_mutex);
}
