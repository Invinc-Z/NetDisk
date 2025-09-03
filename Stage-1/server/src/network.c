#include "../include/network.h"
#include "../include/commons.h"

int tcp_init(const char * ip, int port) {
    //创建套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(listenfd, -1, "socket");

    //设置端口复用
    int opt = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ERROR_CHECK(ret, -1, "setsockopt");
    //绑定地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);
    servaddr.sin_port = htons(port);
    ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    ERROR_CHECK(ret, -1, "bind");
    //监听套接字    
    ret = listen(listenfd, SOMAXCONN);
    ERROR_CHECK(ret, -1, "listen");
    return listenfd;
}

int add_epoll_readfd(int epfd, int fd) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    //设置监听事件
    event.events = EPOLLIN;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
}

int del_epoll_readfd(int epfd, int fd) {
    struct epoll_event event;
    bzero(&event, sizeof(event));
    //设置监听事件
    event.events = EPOLLIN;
    event.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
