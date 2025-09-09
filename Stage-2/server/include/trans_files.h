#ifndef __TRANS_FILES_H__
#define __TRANS_FILES_H__

int sendn(int sockfd, const void * buff, int len);
int recvn(int sockfd, void * buff, int len);

int gets_server(int sockfd, const char * command_content, const char * src_dir);
int puts_server(int sockfd, const char * command_content, const char * src_dir);

#endif /* __TRANS_FILES_H__ */
