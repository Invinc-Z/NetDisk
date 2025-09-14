#include "../include/log.h"
#include "../include/commons.h"
#include "../include/tlc_packet.h"

// 辅助函数：将cmd_type_t 转换为字符串  
const char* cmd2str(cmd_type_t type) {  
    if(type == CHATHEAD) return "chathead";
    else if(type == PWD) return "pwd";
    else if(type == CD) return "cd";
    else if(type == LS) return "ls";
    else if(type == LL) return "ll";
    else if(type == MKDIR) return "mkdir";
    else if(type == RMDIR) return "rmdir";
    else if(type == RM) return "rm";
    else if(type == PUTS) return "puts";
    else if(type == GETS) return "gets";
    else if(type == TREE) return "tree";
    else if(type == CAT) return "cat";
    else return "other";
}

// 客户端断开连接时的日志函数
void log_client_notconnection(int peerfd) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("netdisk-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "client %d is connected at %s", peerfd, time_buffer);
    closelog();
}


// 客户端连接建立时的日志函数
void log_client_connection(int peerfd) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("netdisk-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "client %d is connected at %s ", peerfd, time_buffer);
    closelog();
}

// 客户端用户登录成功时的日志函数
void log_client_user_login(const char* username) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("netdisk-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    syslog(LOG_INFO, "user %s is logined at %s ", username, time_buffer);
    closelog();
}

// 客户端发送消息时的日志函数
void log_client_operation(node_t* p_node) {
    time_t now;
    char time_buffer[80];
    struct tm *timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    openlog("netdisk-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER);
    char cmd[ARR_SIZE + 16] = {0};
    snprintf(cmd, sizeof(cmd), "%s %s",cmd2str(p_node->type), p_node->content);
    syslog(LOG_INFO, "client %d did operations: %s at %s", p_node->netfd, cmd, time_buffer);
    closelog();
}