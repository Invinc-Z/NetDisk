#ifndef __TRANS_FILES_H__
#define __TRANS_FILES_H__

int sendn(int sockfd, const void * buff, int len);
int recvn(int sockfd, void * buff, int len);
int gets_client(int sockfd, const char* filename, const char* download_path);
int puts_client(int sockfd, const char* filename);

#endif /* __TRANS_FILES_H__ */
