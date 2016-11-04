#ifndef __USER_LIST_H__
#define __USER_LIST_H__

int append_user(user_t* user);
user_t* create_user(char* name, uint8_t name_length, int socket);
int remove_user(user_t* user);
int delete_user(user_t* user);

#endif
