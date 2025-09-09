#include "../include/trans_files.h"
#include "../include/commons.h"
#include "../include/tlc_packet.h"

// 发送消息
int sendn(int sockfd, const void * buff, int len)
{
    char * p = (char *)buff;
    int cur_pos = 0;
    while (cur_pos < len) {
        int ret = send(sockfd, p + cur_pos, len - cur_pos, 0);
        if(ret == -1){
            return -1;
        }
        cur_pos += ret;
    }
    return cur_pos;
}

int recvn(int sockfd, void * buff, int len)
{
    char * p = (char *)buff;
    int cur_pos = 0;
    while (cur_pos < len) {
        int ret = recv(sockfd, p + cur_pos, len - cur_pos, 0);
        if(ret == -1){
            return -1;
        }
        else if(ret == 0){
            break;
        }
        cur_pos += ret;
    }
    return cur_pos;
}

int gets_server(int sockfd, const char * command_content, const char * src_dir)
{
    char download_file[ARR_SIZE] = {0};
    strcat(download_file, src_dir);
    strcat(download_file, "/");
    strcat(download_file, command_content);
    printf("download_file = %s\n", download_file);

    // 对文件是否存在进行错误处理
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));

    DIR* dir = opendir(src_dir);
    printf("src_dir = %s\n", src_dir);
    struct dirent * entry;
    struct stat statbuf;
    off_t filesize = 0;
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name, command_content) == 0)
        {
            stat(download_file, &statbuf);
            filesize = statbuf.st_size;
            printf("filesize = %ld\n", filesize);
            break;
        }
    }
    closedir(dir);

    if(filesize == 0) // 文件大小为0或不存在
    {
        packet.type = GETS;
        packet.length = sizeof(filesize);
        memcpy(packet.content, &filesize, packet.length);
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        printf("%s is not exist or file size is 0.\n", download_file);
        return -1;
    }
    packet.type = GETS;
    packet.length = sizeof(filesize);
    memcpy(packet.content, &filesize, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 打开需下载文件的fd
    int fd = open(download_file, O_RDONLY);
    ERROR_CHECK(fd, -1, "open");

    while(1)
    {
        memset(packet.content, 0, sizeof(packet.content));
        packet.length = read(fd, packet.content, sizeof(packet.content));
        if(packet.length > 0)
            sendn(sockfd, (char*)&packet, sizeof(packet.type) + sizeof(packet.length) + packet.length);
        else
            break;
    }
    // 发送结束标志
    packet.length = 0;
    sendn(sockfd, (char*)&packet, sizeof(packet.type) + sizeof(packet.length) + packet.length);
    close(fd);
    printf("send complete!\n\n");
    return 0;
}

int puts_server(int sockfd, const char * command_content, const char * src_dir)
{
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    // 接收文件是否存在错误信息
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);

    if(*(int*)(packet.content) == -1)
    {
        printf("Can not open file\n\n");
        return -1;
    }

    char* upload_filename;
    if((upload_filename = (char*)strrchr(command_content, '/')) == NULL)
    {
        upload_filename = (char*)command_content;
    } 
    // 拼接文件路径和文件名
    char file_name[ARR_SIZE] = {0};
    strcat(file_name, src_dir);
    strcat(file_name, "/");
    strcat(file_name, upload_filename);

    // 打开创建上传文件的fd
    int fd = open(file_name, O_RDWR|O_CREAT,0777);
    ERROR_CHECK(fd,-1,"open");

    // 接收上传文件大小
    off_t filesize = 0;
    memset(&packet, 0, sizeof(packet));
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    filesize = *(int*)packet.content;

    ftruncate(fd, filesize);

    while(1)
    {
        memset(&packet, 0, sizeof(packet));
        recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
        recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
        if(packet.length > 0)
        {
            recvn(sockfd, packet.content, packet.length);
            write(fd, packet.content, packet.length);
        }
        else
        {
            break;
        }
    }
    close(fd);
    printf("recv complete\n\n");
    packet.type = PUTS;
    strcpy(packet.content, "Upload file successfully!\n");
    packet.length = strlen(packet.content);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    return 0;
}
