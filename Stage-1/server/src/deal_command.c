#include "../include/deal_command.h"
#include "../include/trans_files.h"
#include "../include/network.h"
#include "../include/commons.h"

int deal_client_message(block_queue_t* queue, int epfd, int netfd)
{
    // 获取消息类型 消息长度和消息内容
    cmd_type_t type;
    int length = -1;
    int ret = recvn(netfd, &type, sizeof(cmd_type_t));
    ret = recvn(netfd, &length, sizeof(int));

    node_t * p_node = (node_t*)calloc(1, sizeof(node_t));
    p_node->netfd = netfd;
    p_node->epfd = epfd;
    p_node->type = type;

    if(length > 0)
    {
        ret = recvn(netfd, p_node->content, length);
        if(ret > 0){
            // 往线程池中添加任务节点
            if(p_node->type == PUTS){
                // 上传任务 先暂时从epoll实例中删除监听
                del_epoll_readfd(epfd, netfd);                
            }
            queue_enque(queue, p_node); 
        }
    }
    else if(length == 0){ // pwd tree cd等 无参数命令
        queue_enque(queue, p_node);
    }

    if(ret == 0) // 客户端断开
    {  
        printf("\nconn %d is closed.\n", netfd);
        del_epoll_readfd(epfd, netfd);
        close(netfd);
    }
    return 0;
}

// 每个子线程处理任务节点，子线程调用该函数
int do_task(node_t * p_node){
    static char path[ARR_SIZE] = {0};
    char server_path[ARR_SIZE] = {0};
    getcwd(server_path, sizeof(server_path));
    strcat(server_path, "/");
    strcat(server_path, path);

    int ret = 0; // 命令是否执行成功 0表示成功，-1失败
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    switch(p_node->type)
    {
    case CHATHEAD:
        //
        memset(path, 0, sizeof(path));
        memcpy(path, p_node->content, sizeof(p_node->content));  
        break;
    case PWD:   // 发送虚拟路径
        ret = pwd_dir(p_node->netfd, server_path); 
        // 发命令是否执行成功标志给客户端
        packet.type = PWD;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case CD:    // 发送虚拟路径
        ret = cd_dir(p_node->netfd, p_node->content, server_path); 
        // 发命令是否执行成功标志给客户端
        packet.type = CD;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case LS:    // 发送虚拟路径和返回内容
        ret = ls_dir(p_node->netfd, server_path); 
        // 发命令是否执行成功标志给客户端
        packet.type = LS;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case LL:    // 发送虚拟路径和返回内容
        ll_dir(p_node->netfd, server_path);
        // 发命令是否执行成功标志给客户端
        packet.type = LL;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case MKDIR: // 发送虚拟路径
        ret = mk_dir(p_node->netfd, p_node->content, server_path);
        // 发命令是否执行成功标志给客户端
        packet.type = MKDIR;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case RMDIR: // 发送虚拟路径
        ret = rm_dir(p_node->netfd, p_node->content, server_path);
        // 发命令是否执行成功标志给客户端
        packet.type = RMDIR;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case RM: // 发送虚拟路径
        ret = rm_file(p_node->netfd, p_node->content, server_path);
        // 发命令是否执行成功标志给客户端
        packet.type = RM;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case PUTS:
        ret = puts_server(p_node->netfd, p_node->content, server_path);
        // 上传任务完成 添加监听
        add_epoll_readfd(p_node->epfd, p_node->netfd);  
        break;
    case GETS:
        ret = gets_server(p_node->netfd, p_node->content, server_path);
        break;
    case TREE:  // 发送虚拟路径和返回内容
        ret = tree_dir(p_node->netfd, p_node->content, server_path); 
        // 发命令是否执行成功标志给客户端
        packet.type = TREE;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case CAT:   // 发送虚拟路径和返回内容
        ret = cat_file(p_node->netfd, p_node->content, server_path); 
        // 发命令是否执行成功标志给客户端
        packet.type = CAT;
        packet.length = sizeof(ret);
        memcpy(packet.content, &ret, packet.length);
        send(p_node->netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        break;
    case NOTCMD:    
        notcmd(p_node->netfd, server_path);
        printf("命令不合法\n\n");
        break;
    default:
        break;
    }
    return 0;
}
