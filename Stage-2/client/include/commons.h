#ifndef __COMMONS_H__
#define __COMMONS_H__

#include <stdio.h>  //stderr
#include <stdlib.h> //EXIT_FAILURE
#include <stdbool.h>
#include <unistd.h> // close
#include <fcntl.h> // open

#include <sys/stat.h> // fstat stat

// 字符串处理
#include <string.h> // memset

// 网络编程
#include <sys/socket.h> // socket
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // inet_pton   

#include <termios.h>

// 检查命令行参数数量是否符合预期
#define ARGS_CHECK(argc, expected) \
    do { \
        if ((argc) != (expected)) { \
            fprintf(stderr, "args num error!\n");\
            exit(EXIT_FAILURE);\
        } \
    } while (0)                      

// 检查返回值是否是错误标记,若是则打印msg和错误信息
#define ERROR_CHECK(ret, error_flag, msg) \
    do { \
        if ((ret) == (error_flag)) { \
            perror(msg); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#endif /* __COMMONS_H__ */
