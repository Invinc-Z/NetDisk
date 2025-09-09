/*
* @Description: main函数
* @Author: Invinc-Z
*/

#include "../include/commons.h"
#include "../include/tlc_packet.h"
#include "../include/config.h"
#include "../include/threadpool.h"
#include "../include/network.h"
#include "../include/deal_command.h"
#include "../include/login.h"
#define EPOLL_EVENT_SIZE 1024

int exit_pipe[2];

void sig_handler(int num) {
    printf("Parent got a signal, signum = %d\n", num);
    // 激活管道, 往管道中写一个1
    write(exit_pipe[1], "1", 1);
}

// ./server 使用默认的server.conf
// ./server [*.conf] 指定路径的conf
// ./server [server_ip] [server_port] [thread_num]
int main(int argc, char * argv[])
{
    // 读取配置文件
    config_t config;
    memset(&config, 0, sizeof(config));

    if(read_config(argc, argv, &config) < 0) {
        printf("read config failed!\n");
        return -1;
    }

    // 创建匿名管道
    pipe(exit_pipe);

    // fork创建子进程
    if(fork()) {
        // 父进程
        close(exit_pipe[0]);//父进程关闭读端
        signal(SIGUSR1, sig_handler);
        wait(NULL);//等待子进程退出，回收其资源
        printf("\nparent process exit.\n");
        exit(0);//父进程退出
    }

    //子进程
    close(exit_pipe[1]);//子进程关闭写端

    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool)); // 注意线程池结构，队列在栈上，队列中存储的是任务节点的头尾指针
    //初始化线程池
    threadpool_init(&threadpool, config.thread_count);
    //启动线程池
    threadpool_start(&threadpool);

    //创建监听套接字
    int listenfd = tcp_init(config.server_ip, config.port);
    printf("listenfd = %d\n", listenfd);

    // 创建网盘空间
    const char disk_space[ARR_SIZE] = DISK_SPACE;
    mkdir(disk_space, 0777);
    
    //创建epoll实例
    int epfd = epoll_create1(0);

    //对listenfd和父进程发送的信号exit_pipe[0]进行监听
    add_epoll_readfd(epfd, listenfd);
    add_epoll_readfd(epfd, exit_pipe[0]);

    struct epoll_event * p_event_array = (struct epoll_event*)
        calloc(EPOLL_EVENT_SIZE, sizeof(struct epoll_event));
    while(1) {
        int nfds = epoll_wait(epfd, p_event_array, EPOLL_EVENT_SIZE, -1);
        if(nfds == -1)
        {
            if(errno == EINTR)
            {
                printf("EINTR: 被信号中断，继续等待...\n");
                continue;
            }
            else
            {
                perror("epoll_wait 严重错误");
                break;
            }
        }
        for(int i = 0; i < nfds; ++i) {
            if(p_event_array[i].data.fd == listenfd) {
                //有新的客户端连接
                int netfd = accept(listenfd, NULL, NULL);
                ERROR_CHECK(netfd, -1, "accept");
                printf("I am master, I got a netfd = %d\n", netfd);
                while(user_login_verify(netfd) != 0)
                    continue;
                // 将就绪客户端加入监听
                add_epoll_readfd(epfd, netfd);
            }
            else if(p_event_array[i].data.fd == exit_pipe[0]) {
                // 接收到退出信号
                threadpool_stop(&threadpool); // 关闭线程池
                threadpool_destroy(&threadpool); // 销毁线程池
                printf("Master is going to exit.\n");
                exit(0);
            }
            else{
                // 处理已经建立连接的客户端发来的消息
                deal_client_message(&threadpool.queue, epfd, p_event_array[i].data.fd);
            }
        }
    }
    return 0;
}
