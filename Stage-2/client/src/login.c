#include "../include/login.h"
#include "../include/tlc_packet.h"
#include "../include/commons.h"
#include "../include/deal_command.h"
#include "../include/trans_files.h"

int user_login_verify(int sockfd, char* username, int n)
{
    // 1. 终端输入用户名和密码
    // 2. 发送用户名和密码给服务端
    // 3. 接受服务端验证后传回的信号，判断用户名和密码是否正确 

    char* passwd;
    tlc_packet_t packet;

    while(1)
    {
        // 1. 终端输入用户名和密码
        printf("Please enter your username (empty line to quit): ");
        while(s_gets(username, n) == NULL || username[0] == '\0')
        {
            printf("bye\n");
            int flag_login_giveup = 1;
            packet.type = LOGIN_GIVEUP;
            packet.length = sizeof(int);
            memcpy(packet.content, &flag_login_giveup, packet.length);
            send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_WAITALL);
            close(sockfd);
            exit(-1);
        }
        while((passwd = get_passwd("Please enter your password (empty line to quit): ") )== NULL || passwd[0] == '\0')
        {
            printf("bye\n");
            printf("bye\n");
            int flag_login_giveup = 1;
            packet.type = LOGIN_GIVEUP;
            packet.length = sizeof(int);
            memcpy(packet.content, &flag_login_giveup, packet.length);
            send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_WAITALL);
            close(sockfd);
            exit(-1);
        }
        // 2. 发送用户名和密码给服务端
        tlc_packet_t packet;
        memset(&packet, 0, sizeof(packet));
        packet.type = LOGIN_USERNAME;
        packet.length = strlen(username);
        strcpy(packet.content, username);
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_WAITALL);

        memset(&packet, 0, sizeof(packet));
        packet.type = LOGIN_PASSWD;
        packet.length = strlen(passwd);
        strcpy(packet.content, passwd);
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_WAITALL);

        free(passwd);

        // 3. 接受服务端验证后传回的信号，判断用户名和密码是否正确
        cmd_type_t type;
        int length = -1;
        char signal[ARR_SIZE] = {0};

        recvn(sockfd, &type, sizeof(cmd_type_t));
        recvn(sockfd, &length, sizeof(int));
        recvn(sockfd, signal, length);
        int flag = *(int*)signal;
        if(flag == 0)
        {
            printf("Welcome to NetDisk!\n");
            break;
        }
        else if(flag == 1)
        {
            printf("password is not correct.\n");
        }
        else if(flag == -1)
        {
            printf("username is not exist.\n");
        }
    }
    return 0;
}

void set_echo(bool enable)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if(enable) tty.c_lflag |= ECHO;
    else tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

char* get_passwd(const char* prompt)
{
    printf("%s", prompt);
    set_echo(false); // 关闭回显
    char* passwd = (char*)malloc(PASSWD_LEN);
    s_gets(passwd, PASSWD_LEN);
    set_echo(true); // 恢复回显
    printf("\n"); //关闭辉县时换行符不显示，手动补一个
    return passwd;
}
