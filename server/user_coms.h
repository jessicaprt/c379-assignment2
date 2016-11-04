#ifndef __USER_COMS_H__
#define __USER_COMS_H__

int send_user_list(int socket);
void broadcast_user_join(user_t* user);
void broadcast_user_quit(user_t* user);
void broadcast_msg(user_t* user, uint16_t msg_length, char* msg);

#endif
