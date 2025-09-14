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

    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));

    // 1. 发送文件是否存在标志
    int file_error_flag = 0; // 0 文件可以打开 -1 文件无法打开
    packet.type = GETS;
    // 可读打开上传文件
    int fd = open(download_file, O_RDONLY);
    if(fd == -1)
    {
        // 文件不存在 发送错误标志
        file_error_flag = -1;
        packet.length = sizeof(file_error_flag);
        memcpy(packet.content, &file_error_flag, packet.length);
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        printf("Can not open file %s\n", download_file);
        return -1;
    }

    // 文件存在 发送正确标志 
    packet.length = sizeof(file_error_flag);
    memcpy(packet.content, &file_error_flag, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    printf("open file %s succeed.\n", download_file);

    // 2. 发送文件大小
    struct stat statbuf;
    off_t filesize = 0;
    stat(download_file, &statbuf);
    filesize = statbuf.st_size;
    printf("filesize = %ld\n", filesize);

    memset(&packet, 0, sizeof(packet));
    packet.type = GETS;
    packet.length = sizeof(filesize);
    memcpy(packet.content, &filesize, packet.length);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 3. 接收客户端文件已存在的大小
    int exist_filesize = 0;
    memset(&packet, 0, sizeof(packet));
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    exist_filesize = *(int*)packet.content;

    if(exist_filesize > 0)
    {
        lseek(fd, exist_filesize, SEEK_SET);
    }

    // 4. 发送文件
    if(filesize > 1024 * 1024 * 100) // > 100M mmap
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        off_t aligned_exist_size = (exist_filesize / page_size) * page_size;
        off_t offset_in_page = exist_filesize - aligned_exist_size;

        // 实际映射长度
        size_t map_length = filesize - aligned_exist_size;

        char* map = (char*)mmap(NULL, map_length, PROT_READ, MAP_SHARED, fd, aligned_exist_size);
        sendn(sockfd, map + offset_in_page, filesize - exist_filesize);
        munmap(map, map_length);
    }
    else{
        while(1)
        {

            memset(packet.content, 0, sizeof(packet.content));
            packet.length = read(fd, packet.content, sizeof(packet.content));
            if(packet.length > 0)
                send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
            else
                break;
        }
        // 发送结束标志
        packet.length = 0;
        send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    }
    close(fd);
    printf("send complete!\n");
    packet.type = GETS;
    strcpy(packet.content, "Download file successfully!\n");
    packet.length = strlen(packet.content);
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    return 0;
}

int puts_server(int sockfd, const char * command_content, const char * src_dir)
{
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    // 1. 接收文件是否存在错误信息
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

    // 2. 发送文件字节数，不存在发送0
    int exist_filesize = 0;
    struct stat statbuf;
    if(stat(file_name, &statbuf) == 0)
        exist_filesize = statbuf.st_size;
    else
        exist_filesize = 0;
    memset(&packet, 0, sizeof(packet));
    packet.type = PUTS;
    packet.length = sizeof(exist_filesize);
    memcpy(packet.content, &exist_filesize, sizeof(exist_filesize));
    send(sockfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);


    // 打开创建上传文件的fd
    int fd = open(file_name, O_RDWR|O_CREAT,0777);
    ERROR_CHECK(fd,-1,"open");

    // 3. 接收上传文件大小
    off_t filesize = 0;
    memset(&packet, 0, sizeof(packet));
    recvn(sockfd, (char*)&packet.type, sizeof(packet.type));
    recvn(sockfd, (char*)&packet.length, sizeof(packet.length));
    recvn(sockfd, packet.content, packet.length);
    filesize = *(int*)packet.content;

    if(exist_filesize > 0)
    {
        lseek(fd, exist_filesize, SEEK_SET);
    }
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
            {
                break;
            }
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
