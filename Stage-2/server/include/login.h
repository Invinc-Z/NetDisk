#ifndef __LOGIN_H__
#define __LOGIN_H__

int user_login_verify(int netfd);
void get_salt(char* salt, const char* passwd);
int verify_username_passwd(const char* username, const char* passwd);

#endif /* __LOGIN_H__ */
