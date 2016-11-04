#ifndef __USER_LIST_H__
#define __USER_LIST_H__

extern user_t * user_list_head;
extern user_t * user_list_tail;
extern uint16_t user_list_length;


int append_user(user_t* user);
user_t* create_user(char* name, uint8_t name_length, int socket);
int remove_user(user_t* user);
int delete_user(user_t* user);
int user_list_lock();
int user_list_unlock();

#endif
