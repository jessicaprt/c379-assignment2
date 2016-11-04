CC=gcc
DEPS= servcli.h
 
all: server379 chat379
 
server379: server/server.c server/daemonize.c server/main_events.c server/open_main_fd.c server/running.c server/user_coms.c server/user_handler.c server/user_list.c
    gcc -o server379 server/server.c server/daemonize.c server/main_events.c server/open_main_fd.c server/running.c server/user_coms.c server/user_handler.c server/user_list.c
    mkdir -p bin
    mv server379 bin/
 
chat379: client/client.c
    gcc -o chat379 client/client.c
    mkdir -p bin
    mv chat379 bin/