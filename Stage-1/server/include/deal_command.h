#ifndef __DEAL_COMMAND_H__
#define __DEAL_COMMAND_H__
#include "block_queue.h"

int deal_client_message(block_queue_t * queue, int epfd, int netfd);
int do_task(node_t * p_node); 

int pwd_dir(int netfd, const char * src_dir); // pwd命令
int cd_dir(int netfd, const char * command_content, const char * src_dir); // cd命令
int ls_dir(int netfd, const char * src_dir); // ls命令
int ll_dir(int netfd, const char * src_dir); // ll命令

int mk_dir(int netfd, const char * command_content, const char * src_dir); // mkdir命令
int rm_dir(int netfd, const char * command_content, const char * src_dir); // rmdir命令  
int rm_file(int netfd, const char * command_content, const char * src_dir); // rm命令
int tree_dir(int netfd, const char * command_content, const char * src_dir); // tree命令                                                                  
int cat_file(int netfd, const char * command_content, const char * src_dir); // cat命令                                                                           
int notcmd(int netfd, const char * src_dir); // notcmd命令
#endif /* __DEAL_COMMAND_H__ */
