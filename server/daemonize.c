#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "server.h"

int daemonize() {
    // Returns pid of log file or exits on failure
  
    pid_t pid = fork();
    pid_t sid = 0;
    int fp = 0;

    if (pid < 0){
        fprintf(stderr, "Unable to fork child process\n");
        exit(EXIT_FAILURE);
    }

    if (pid > 0){
        fprintf(stderr, "Server is continuing to run on PID %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    pid = getpid();

    char filename[255];
    sprintf(filename, "server379%d.log", pid);

    log_stream = fopen(filename, "w+");
    if(log_stream == NULL){
        fprintf(stderr, "Unable to open log stream\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "Writting logs to file: \"%s\"\n", filename);

    errno = 0;
    int status = fprintf(log_stream, "Starting logs\n");
    int err = errno;
    if (status < 0){
        fprintf(stderr, "Unable to write to the file: \"%s\"\n", filename);
        fprintf(stderr, "%s, %p\n", strerror(err), log_stream);
        exit(EXIT_FAILURE);
    }
    fflush(log_stream);

    fprintf(log_stream, "Server PID: %d\n", pid);
    fflush(log_stream);


    sid = setsid();
    if(sid < 0){
        fprintf(stderr, "Unable to create new process group.\n");
        exit(EXIT_FAILURE);
    }

    if(chdir("/") < 0) {
        fprintf(stderr, "Unable to change working directory to \"/\"\n");
        exit(EXIT_FAILURE);
    }
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return fileno(log_stream);
}

