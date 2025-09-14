#ifndef __LOG_H__
#define __LOG_H__
#include "block_queue.h"

// 客户端断开连接时的日志函数
void log_client_notconnection(int peerfd);

// 客户端连接建立时的日志函数
void log_client_connection(int peerfd);

void log_client_user_login(const char* username);

// 假设的客户端发送消息时的日志函数
void log_client_operation(node_t* p_node);

#endif /* __LOG_H__ */