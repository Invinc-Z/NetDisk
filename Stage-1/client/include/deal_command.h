#ifndef __DEAL_COMMAND_H__
#define __DEAL_COMMAND_H__
#include "tlc_packet.h"

#define HOME "MyDisk"
#define DOWNLOAD_PATH "Downloads"

char * s_gets(char * st, int n); // 读取前n个字符，若多于n舍弃多余字符，少于n时删除最后的换行符
cmd_type_t str2cmd(const char* cmd);
int deal_command(int sockfd);

#endif /* __DEAL_COMMAND_H__ */
