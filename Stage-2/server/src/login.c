#include "../include/login.h"
#include "../include/tlc_packet.h"
#include "../include/commons.h"
#include "../include/trans_files.h"

int user_login_verify(int netfd)
{
    // 1. 接收客户端发来的用户名和密码
    // 2. 与服务器的用户系统中的用户名和密码进行比较
    // 3. 回传信号给客户端 [0 输入正确] [1 用户名正确 密码错误] [-1 用户名错误]

    cmd_type_t type;
    int ret = 0;
    int length = -1;
    char username[NAME_LEN] = {0};
    char passwd[PASSWD_LEN] = {0};

     // 1. 接收客户端发来的用户名和密码
    ret = recvn(netfd, &type, sizeof(cmd_type_t));
    // 客户端放弃登录
    if(type == LOGIN_GIVEUP || ret == 0)
    {
        char flag_login_giveup[ARR_SIZE] = {0};
        recvn(netfd, &length, sizeof(int));
        recvn(netfd, flag_login_giveup, length);
        printf("\nnetfd %d is closed.\n", netfd);
        close(netfd);
        return 0;
    }
    recvn(netfd, &length, sizeof(int));
    recvn(netfd, username, length);

    recvn(netfd, &type, sizeof(cmd_type_t));
    recvn(netfd, &length, sizeof(int));
    recvn(netfd, passwd, length);

    // 2. 与服务器的用户系统中的用户名和密码进行比较
    ret = verify_username_passwd(username, passwd);

    // 3. 回传信号给客户端 [0 输入正确] [1 用户名正确 密码错误] [-1 用户名错误]
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = LOGIN_USERNAME;
    packet.length = sizeof(ret);
    memcpy(packet.content, &ret, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    return ret;
}

void get_salt(char* salt, const char* passwd)
{
    int i, j;
    for(i = 0, j = 0; passwd[i] && j < 3; ++i)
    {
        if(passwd[i] == '$')
            j++;
    }
    strncpy(salt, passwd, i + 1);
}

int verify_username_passwd(const char* username, const char* passwd)
{
    struct spwd* sp;
    char salt[512] = {0};
    // 获取当前执行进程的实际上用户id，保存便于后面恢复
    uid_t uid = getuid();

    // 当前设置用户ID位已经打开，且程序文件所有者为root
    // 所以当前进程的有效用户为root，具有超级用户权限，开绕过getspnam的权限检查

    // 检查用户名是否存在
    if((sp = getspnam(username)) == NULL)
    {
        printf("user %s is not exist!\n", username);
        return -1;
    }

    // 进程具有超级用户特权，setuid将实际用户ID、有效用户ID、保存的设置用户ID设置为uid 取消超级用户权限
    if(setuid(uid) != 0)
        perror("setuid");

    // 获取盐值
    get_salt(salt, sp->sp_pwdp);

    if(strcmp(sp->sp_pwdp, crypt(passwd, salt)) == 0)
    {
        printf("password is correct.\n");
        // 在这里创建家目录
        char home[ARR_SIZE] = {0};
        strcat(home, "./");
        strcat(home, DISK_SPACE);
        strcat(home, "/");
        strcat(home, username);
        if(mkdir(home, 0755) != 0 && errno != EEXIST)
        {
            printf("Failed to create %s directory\n", home);
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    else
    {
        printf("password is not correct.\n");
        return 1;
    }
}