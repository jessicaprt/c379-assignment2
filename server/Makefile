all: server379

server379: server.c daemonize.c open_main_fd.c running.c main_events.c \
	user_handler.c user_list.c user_coms.c
	gcc -o server379 -pthread $^

#server379: server.h user.h user_coms.h user_list.h
