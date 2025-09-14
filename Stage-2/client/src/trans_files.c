#include "../include/trans_files.h"
#include "../include/commons.h"
#include "../include/tlc_packet.h"
#include "../include/deal_command.h"

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

int gets_client(int sockfd, const char* filename)
{
    // 初始化packet
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));

    // 1. 接收标志,文件是否存在错误信息
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    if(*(int*)(packet.content) == -1)
    {   // 文件不存在
        printf("Can not open file\n\n");
        return -1;
    }
    
    // 文件存在
    // 2. 接收下载文件大小
    off_t filesize = 0;
    memset(&packet, 0, sizeof(packet));
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    filesize = *(int*)packet.content;

    // 3. 发送已存在文件的大小

    // 拼接文件路径和文件名
    char *cwd = getcwd(NULL, 0);
    char file_path[ARR_SIZE] = {0};
    char download_home[ARR_SIZE] = {0};
    strcat(file_path, cwd);
    strcat(file_path, "/");
    strcat(file_path, DOWNLOAD_PATH);
    strcpy(download_home, file_path);
    strcat(file_path, "/");
    strcat(file_path, filename);
    free(cwd); // 必须释放内存

    // 打开创建上传文件的fd
    umask(0);
    mkdir(download_home, 0775);
    int fd = open(file_path, O_RDWR|O_CREAT,0666);
    ERROR_CHECK(fd, -1, "open");

    // 发送已经存在文件字节数，不存在发送0
    int exist_filesize = 0;
    struct stat statbuf;
    if(fstat(fd, &statbuf) == 0)
        exist_filesize = statbuf.st_size;
    else
        exist_filesize = 0;
    memset(&packet, 0, sizeof(packet));
    packet.type = GETS;
    packet.length = sizeof(exist_filesize);
    memcpy(packet.content, &exist_filesize, sizeof(exist_filesize));
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 重定位文件读写偏移量   
    if(exist_filesize > 0)
    {
        lseek(fd, exist_filesize, SEEK_SET);
    }
    // 为内存映射（mmap）准备文件大小
    ftruncate(fd, filesize);

    // 4. 接收文件
    if(filesize > 1024 * 1024 * 100)
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        off_t aligned_exist_size = (exist_filesize / page_size) * page_size;
        off_t offset_in_page = exist_filesize - aligned_exist_size;

        // 实际映射长度
        size_t map_length = filesize - aligned_exist_size;
        char* map = (char*)mmap(NULL, map_length, PROT_WRITE, MAP_SHARED, fd, aligned_exist_size);
        recvn(sockfd, map + offset_in_page, filesize - exist_filesize);
        munmap(map, map_length);
    }
    else{
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
                break;
        }
    }
    
    close(fd);
    printf("recv complete\n\n");
    return 0;
}

int puts_client(int sockfd, const char* filename)
{
    int file_error_flag = 0; // 0 文件可以打开 -1 文件无法打开
    // 初始化packet
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    // 1. 发送文件是否存在标志
    packet.type = PUTS;
    // 可读打开上传文件
    int fd = open(filename, O_RDONLY);
    if(fd == -1)
    {
        // 文件不存在 发送错误标志
        file_error_flag = -1;
        packet.length = sizeof(file_error_flag);
        memcpy(packet.content, &file_error_flag, packet.length);
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        printf("Can not open file %s\n", filename);
        return -1;
    }

    // 文件存在 发送正确标志 
    file_error_flag = 0;
    packet.length = sizeof(file_error_flag);
    memcpy(packet.content, &file_error_flag, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    printf("open file %s succeed.\n", filename);

    // 2. 接收服务端已有文件字节数
    int exist_filesize = 0;
    memset(&packet, 0, sizeof(packet));
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    exist_filesize = *(int*)packet.content;

    
    // 获取上传文件大小
    struct stat statbuf;
    memset(&statbuf, 0, sizeof(statbuf));
    fstat(fd, &statbuf);
    off_t filesize = statbuf.st_size;

    // 3. 发送文件大小
    memset(packet.content, 0, sizeof(packet.content));
    packet.length = sizeof(filesize);
    memcpy(packet.content, &filesize, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(exist_filesize > 0)
    {
        lseek(fd, exist_filesize, SEEK_SET);
    }

    // 4. 发送文件
    if(filesize > 1024 * 1024 * 100)  // > 100M mmap
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        off_t aligned_exist_size = (exist_filesize / page_size) * page_size;
        off_t offset_in_page = exist_filesize - aligned_exist_size;

        // 实际映射长度
        size_t map_length = filesize - aligned_exist_size;

        char* map = (char*)mmap(NULL, map_length, PROT_READ, MAP_PRIVATE, fd, aligned_exist_size);
        sendn(sockfd, map + offset_in_page, filesize - exist_filesize);
        munmap(map, map_length);
    }
    else{
        while(1)
        {
            memset(packet.content, 0, sizeof(packet.content));
            packet.length = read(fd, packet.content, sizeof(packet.content));
            if(packet.length > 0)
            {
                send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
            }
            else
                break;
        }
        // 发送结束标志
        packet.length = 0;
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    }

    close(fd);
    printf("send complete!\n");
    return 0;
}
