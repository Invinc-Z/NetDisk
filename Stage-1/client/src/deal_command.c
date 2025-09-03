#include "../include/deal_command.h"
#include "../include/commons.h"
#include "../include/trans_files.h"

int deal_command(int sockfd) {
    // 设置聊天头
    static char chat_head[ARR_SIZE] = {0};
    static const char home[ARR_SIZE] = HOME;
    for(size_t i = 0; i < strlen(home); i++)
        chat_head[i] = home[i];
    printf("> %s: ", chat_head);
    fflush(stdout);

    char input[ARR_SIZE] = {0};         // 接收客户端输入
    char command[ARR_SIZE] = {0};       // 接收命令
    char parameter[ARR_SIZE] = {0};     // 接收参数
    char strtok_input[ARR_SIZE] = {0};  // 用于分割命令和参数

    // 获取客户端输入
    s_gets(input, sizeof(input));

    // 复制输入命令
    memcpy(strtok_input, input, sizeof(input));
    const char delimiters[] = " "; // 截取依据
                                   // 截取命令
    char * token = strtok(strtok_input, delimiters);
    memcpy(command, token, strlen(token));
    // 截取参数
    token = strtok(NULL, delimiters);
    if(token){
        memcpy(parameter, token, strlen(token));
    }
    // 初始化发送包
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));

    // 发送路径(聊天头)
    packet.type = CHATHEAD;
    packet.length = strlen(chat_head);
    memcpy(packet.content, chat_head, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_WAITALL);

    // 发送命令和命令参数
    packet.type = str2cmd(command);
    packet.length = strlen(parameter);
    memcpy(packet.content, parameter, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(packet.type == PUTS)  
    {
        if (puts_client(sockfd, packet.content) == -1)
            return -1;
    }

    char route[ARR_SIZE] = {0};     // 接收虚拟路径
    char content[ARR_SIZE] = {0};   // 接收返回内容
    char mark[ARR_SIZE] = {0};      // 接收命令是否正确执行的标志，flag的字节表示
    int flag = 0;                   // 接收命令是否正确执行的标志，转换mark用

    memset(&packet, 0, sizeof(packet));
    // 接收类型
    recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
    switch(packet.type)
    {
    case PWD:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            printf("%s\n", route);
        }
        else{
            printf("pwd false!\n");
        }
        break;
    case CD:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            memcpy(chat_head, route, strlen(route));
            chat_head[strlen(route)] = 0; // 聊天头变短时加结束符
        }
        else{
            printf("cd false!\n");
        }
        break;
    case LS:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收内容
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(content, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(strlen(content) == 0) return 0;
        if(flag == 0){
            printf("%s\n", content);
        }
        else{
            printf("ls false!\n");
        }
        break;
    case LL:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收内容
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(content, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(strlen(content) == 0) return 0;
        if(flag == 0){
            printf("%s", content);
        }
        else{
            printf("ll false!\n");
        }
        break;
    case MKDIR:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            // printf("flag = %d\n", flag); // 打印标志
        }
        else{
            printf("mkdir false!\n");
        }
        break;
    case RMDIR:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            // printf("flag = %d\n", flag); // 打印标志
        }
        else{
            printf("rmdir false!\n");
        }
        break;
    case RM:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(route, packet.content, packet.length);
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            // printf("flag = %d\n", flag); // 打印标志
        }
        else{
            printf("rm false!\n");
        }
        break;
    case PUTS:
        recvn(sockfd, &packet.length, sizeof(packet.length));
        recvn(sockfd, packet.content, packet.length);
        printf("%s\n", packet.content);
        break;
    case GETS:
        gets_client(sockfd, parameter, DOWNLOAD_PATH);
        break;
    case TREE:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        // 接收内容
        while(1)
        {
            recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
            recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
            memset(packet.content, 0, sizeof(packet.content));
            recv(sockfd, packet.content, packet.length, MSG_WAITALL);
            memcpy(content, packet.content, packet.length);
            if(packet.length < ARR_SIZE)
            {
                printf("%s", content);
                break;
            }
            printf("%s", content);
        }
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            /* printf("cat true\n"); */
        }
        else{
            printf("tree false!\n");
        }
        break;
    case CAT:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        // 接收内容
        while(1)
        {
            recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
            recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
            memset(packet.content, 0, sizeof(packet.content));
            recv(sockfd, packet.content, packet.length, MSG_WAITALL);
            memcpy(content, packet.content, packet.length);
            printf("%s", content);
            if(packet.length < ARR_SIZE) break;
        }
        // 接收标志
        recv(sockfd, &packet.type, sizeof(packet.type), MSG_WAITALL);
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        memcpy(mark, packet.content, packet.length);
        flag = *(int*)mark;
        if(flag == 0){
            /* printf("cat true\n"); */
        }
        else{
            printf("cat false!\n");
        }
        break;
    case NOTCMD:
        // 接收路径
        recv(sockfd, &packet.length, sizeof(packet.length), MSG_WAITALL);
        recv(sockfd, packet.content, packet.length, MSG_WAITALL);
        printf("命令不合法！\n");
        break;
    default:
        break;
    }
    return 0;
}

char* s_gets(char* st, int n)
{
    char* find;
    char* ret_val = fgets(st, n, stdin);
    if(ret_val)
    {
        find = strchr(st,'\n');
        if(find) *find = '\0';
        else 
        {
            while(getchar()!='\n');
        }
    }
    return ret_val;
}

cmd_type_t str2cmd(const char* cmd)
{
    if(strcmp(cmd,"pwd") == 0) return PWD;
    else if(strcmp(cmd, "cd") == 0) return CD;
    else if(strcmp(cmd, "ls") == 0) return LS;
    else if(strcmp(cmd, "ll") == 0) return LL;
    else if(strcmp(cmd, "mkdir") == 0) return MKDIR;
    else if(strcmp(cmd, "rmdir") == 0) return RMDIR;
    else if(strcmp(cmd, "rm") == 0) return RM;
    else if(strcmp(cmd, "puts") == 0) return PUTS;
    else if(strcmp(cmd, "gets") == 0) return GETS;
    else if(strcmp(cmd, "tree") == 0) return TREE;
    else if(strcmp(cmd, "cat") == 0) return CAT;
    else return NOTCMD;
}
