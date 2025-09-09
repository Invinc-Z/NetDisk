/*
 * @Description: main
 * @Author: Invinc-Z
 */
#include "../include/tlc_packet.h"
#include "../include/commons.h"
#include "../include/network.h"
#include "../include/deal_command.h"
#include "../include/login.h"

/* Usage: ./client [server_ip] [server_port]*/
int main(int argc, char *argv[]) {
    // 检查命令行参数数量
    ARGS_CHECK(argc,3);

    // 连接服务器
    int sockfd = tcp_connect(argv[1], argv[2]);
    printf("Connect is successful! sockfd = %d\n\n", sockfd);

    // 用户登录验证
    char username[NAME_LEN] = {0}; 
    user_login_verify(sockfd, username, NAME_LEN); // username 传出用作建立用户家目录

    // 循环处理
    while(1)
    {
        deal_command(sockfd, username);
    }
    close(sockfd);
    return 0;
}            
