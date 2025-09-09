#ifndef __LOGIN_H__
#define __LOGIN_H__
#include <stdbool.h>

int user_login_verify(int sockfd, char* username, int n); 
void set_echo(bool enable);
char* get_passwd(const char* prompt);


#endif /* __LOGIN_H__ */
