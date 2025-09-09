#include "../include/network.h"
#include "../include/commons.h"

int tcp_connect(const char * ip, const char * port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sockfd, -1, "socket");
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, ip, &serveraddr.sin_addr.s_addr);
    
    int ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    ERROR_CHECK(ret, -1, "connect");
    
    return sockfd;
}
