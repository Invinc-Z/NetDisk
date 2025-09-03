#include "../include/deal_command.h"
#include "../include/commons.h"
#include "../include/stack.h"
#include "../include/tlc_packet.h"

// cd_dir专用, 得到最简路径的路径栈
static int get_path_stack(linked_stack_t * p_stack, char * p) {
    // .. 弹栈, 判空   .不处理   普通的压栈
    if(strcmp(p, "..") == 0) {
        char data[ARR_SIZE] = {0};
        int ret = stack_pop(p_stack, data);
        if(ret == -1) {
            return -1;
        }
    }
    else if(strcmp(p, ".") == 0) {
        return 1;
    }
    else 
        stack_push(p_stack, p);
    return 1;
}

// cd_dir专用, 用于将路径栈弹出, 得到最简路径, 进入此函数已代表栈非空
static void get_fin_fake_path(linked_stack_t * p_stack, char * dest_dir) {
    char dest_dir_temp[ARR_SIZE] = {0}; 

    while(!stack_is_empty(p_stack)) {
        // 把栈顶数据取出来
        char data[ARR_SIZE] = {0};
        stack_pop(p_stack, data);

        // 将取出的文件夹名放入总字符串, 后取出的放前面
        char temp[ARR_SIZE] = {0};
        strcpy(temp, data);
        // 给每个路径文件名后加'/', 第一个(路径的最后一个)后不加
        if(dest_dir_temp[0] != 0) {
            strcat(temp, "/");
            strcat(temp, dest_dir_temp);
        }
        strcpy(dest_dir_temp, temp);
    }
    strcpy(dest_dir, dest_dir_temp);
}

// cd_dir专用, 得到两个路径的不同之处 path2-path1
static int get_path_diff(const char *path1, const char *path2, char *diff) {
    // 找出两个路径共同的前缀长度
    int common_len = 0;
    while ((path1[common_len] != '\0') && 
           (path2[common_len] != '\0') && 
           (path1[common_len] == path2[common_len])) {
        common_len++;
    }

    // 如果完全匹配，返回0
    if ((path1[common_len] == '\0') && (path2[common_len] == '\0')) {
        return 0;
    }

    // 复制第二个路径中不同的部分到结果
    const char *diff_part = path2 + common_len + 1; // +1 表示跳过斜杠
    int diff_len = strlen(diff_part);

    // 确保不超过结果缓冲区大小（假设result足够大）
    strncpy(diff, diff_part, diff_len);
    diff[diff_len] = '\0';

    // 1表示有不同部分
    return 1;
}

int pwd_dir(int netfd, const char *src_dir) {
    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    printf("real_dir = %s\n", real_dir);
    printf("src_dir = %s\n", src_dir);
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = PWD;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    printf("pwd之后的路径为: \n%s\n\n", diff_dir);
    return 0;
}

int cd_dir(int netfd, const char * command_content, const char * src_dir) {
    int return_value = 0;
    // 将要打开的目录拼接成真实地址
    char dest_dir[ARR_SIZE] = {0};
    strcpy(dest_dir, src_dir); // dest_dir: 真实地址(当前工作目录+虚拟用户路径)

    // 下面要考虑复杂文件路径的问题, 化成最简单的, 再拼接
    // command_content:  ../dir1/../dir2/./dir1/../dir1  --> /dir2/dir1

    // 目的：进行入栈, 化为最简进入路径
    // 先得到服务端终端路径real_dir, 即源地址  e.g. real_dir: /home/group
    // 再将源地址与src_dir对比, 得到多的部分        src_dir: /home/group/xjh
    // 多的部分与command_content拼接                       xjh 拼接 command_content
    // 按照入栈规则判断, 得到最终栈

    // 将最终栈依次弹出, 加到最前面, 得dest_dir_temp    xjh/dir2/dir1
    // 将dest_dir_temp与real_dir拼接, 得到最终绝对路径dest_dir   /home/group/xjh/dir2/dir1
    // 检查路径是否存在, 存在则发送, 不存在则失败

    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    printf("real_dir is: %s\n", real_dir);

    char diff_dir[ARR_SIZE] = {0};
    int ret = get_path_diff(real_dir, src_dir, diff_dir);

    strcat(diff_dir, "/");
    strcat(diff_dir, command_content);
    printf("waiting for opearte path is: %s\n", diff_dir);

    linked_stack_t * p_stack = stack_create();
    char *p = strtok(diff_dir, "/");
    do {
        ret = get_path_stack(p_stack, p);
        if(ret == -1) {
            printf("cd_dir无法打开更高级目录\n");
            return_value = -1;
            break;
        }
        p = strtok(NULL, "/");
    } while(p != NULL);

    char dest_dir_temp[ARR_SIZE] = {0};
    if(return_value == 0) {
        get_fin_fake_path(p_stack, dest_dir_temp);
        if(dest_dir_temp[0] == 0) {
            printf("get_fin_fake_path is NULL\n");
            return_value = -1;
        }
        else {
            printf("get_fin_fake_path is %s\n", dest_dir_temp);
        }

        stack_destroy(p_stack);

        char dest_dir_save[ARR_SIZE] = {0};
        strcpy(dest_dir_save, real_dir);
        strcat(dest_dir_save, "/");
        strcat(dest_dir_save, dest_dir_temp);

        strcpy(dest_dir, dest_dir_save);
        if(access(dest_dir, F_OK) != 0) {
            printf("cd_dir路径不存在\n");
            return_value = -1;
        }
        else {
            printf("real dest_dir is %s\n", dest_dir);   
        }
    }

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = CD;
    packet.length = strlen(dest_dir_temp);
    memcpy(packet.content, dest_dir_temp, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    printf("cd之后的路径为: %s\n\n", dest_dir_temp);

    if(return_value == -1) {
        return -1;
    }
    return 0;
}

int ls_dir(int netfd, const char * src_dir) {
    // 初始化发送给客户端的响应
    char content[ARR_SIZE] = {0};

    // 对目录里文件进行遍历
    DIR * dirp = opendir(src_dir);
    struct dirent * pdirent = {0};
    while((pdirent = readdir(dirp))) {
        // 跳过.和..目录
        if(strcmp(pdirent->d_name, ".") == 0 || strcmp(pdirent->d_name, "..") == 0) {
            continue;
        }
        // 拼接文件名
        strcat(content, pdirent->d_name);
        if(pdirent->d_type == DT_DIR) {
            strcat(content, "/"); // 如果是目录，后面加上/
        }
        strcat(content, " "); // 每项后面加上空格分隔
    }
    closedir(dirp); // 关闭目录

    // 获取当前虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = LS;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 发送响应信息给客户端
    packet.length = strlen(content);
    memset(packet.content, 0, sizeof(packet.content));
    memcpy(packet.content, content, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    printf("ls之后的路径为: %s\n", diff_dir);
    printf("ls的内容为: %s\n\n", content);
    return 0;
}

int ll_dir(int netfd, const char * src_dir) {
    // 获取当前虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = LL;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 初始化发送给客户端的响应
    char content[ARR_SIZE] = {0};

    // 对目录里文件进行遍历
    DIR * dirp = opendir(src_dir);
    struct dirent * pdirent = {0};
    struct stat file_info;
    while((pdirent = readdir(dirp))) {
        // 跳过.和..目录
        if(strcmp(pdirent->d_name, ".") == 0 || strcmp(pdirent->d_name, "..") == 0) {
            continue;
        }
        char fullpath[300];
        snprintf(fullpath, sizeof(fullpath),"%s%s%s", src_dir, "/", pdirent->d_name);
        stat(fullpath, &file_info);
        char perm[2];
        switch (file_info.st_mode & S_IFMT) {
        case S_IFBLK: perm[0]='b';   break;
        case S_IFCHR: perm[0]='c';   break;
        case S_IFDIR: perm[0]='d';   break;
        case S_IFIFO: perm[0]='p';   break;
        case S_IFLNK: perm[0]='l';   break;
        case S_IFREG: perm[0]='-';   break;
        case S_IFSOCK:perm[0]='s';   break;
        default:      perm[0]='u';   break;
        }
        perm[1] = '\0';
        // 拼接文件名
        char filename[64] = {0};
        strcpy(filename, pdirent->d_name);
        if(pdirent->d_type == DT_DIR) {
            strcat(filename, "/"); // 如果是目录，后面加上/
        }
        char time_str[64];
        char one_file[256];
        strftime(time_str, sizeof(time_str),"%Y-%m-%d %H:%M",localtime(&file_info.st_mtime));
        sprintf(one_file, "%s %16ld %s %s\n",perm, file_info.st_size, time_str, filename);
        strcat(content, one_file);
    }
    closedir(dirp); // 关闭目录

    // 要么设置每个文件作为一个包发送，这样每个文件浪费8个字节，要么放在一起发送，这种情况需要包的content字段足够大
    // 这里选择放在一起发送

    // 发送响应信息给客户端
    packet.length = strlen(content);
    memset(packet.content, 0, sizeof(packet.content));
    memcpy(packet.content, content, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    printf("ll之后的路径为: %s\n", diff_dir);
    printf("ll的内容为: \n%s\n\n", content);
    return 0;
}

int mk_dir(int netfd, const char * new_dir, const char * src_dir){
    int return_value = 0;
    char dir[ARR_SIZE] = {0};
    strcpy(dir, new_dir);

    char path[ARR_SIZE] = {0};
    strcpy(path, src_dir);

    strcat(path, "/");
    strcat(path, dir);

    if(mkdir(path, 0755) == 0){
        printf("成功创建目录：%s\n\n", dir);
    }
    else{
        printf("创建目录失败\n\n");
        return_value = -1;
    }

    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = MKDIR;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(return_value == -1){
        return -1;
    }
    return 0;
}

int rm_dir(int netfd, const char * del_dir, const char * src_dir){
    int return_value = 0;

    // 拼接成完整地址
    char dest_dir[ARR_SIZE] = {0};
    strcat(dest_dir, src_dir);
    strcat(dest_dir, "/");
    strcat(dest_dir, del_dir);

    int ret = rmdir(dest_dir);
    if(ret == -1){
        printf("rmdir文件夹非空或不存在\n");
        return_value = -1;
    }


    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = RMDIR;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(return_value == -1){
        return -1;
    }
    return 0;
}


int rm_file(int netfd, const char * del_file, const char * src_dir){
    int return_value = 0;
    char file[ARR_SIZE] = {0};
    strcpy(file, del_file);

    char path[ARR_SIZE] = {0};
    strcpy(path, src_dir);

    strcat(path, "/");
    strcat(path, file);

    if(unlink(path) == 0){
        printf("成功删除文件：%s\n\n", file);
    }
    else{
        printf("删除文件失败\n\n");
        return_value = -1;
    }

    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = RM;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(return_value == -1){
        return -1;
    }
    return 0;
}

static void listFilesRecursively(const char *basePath, int depth, char * content) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileInfo;
    char path[1024] = {0};
    char line[1024] = {0};  // 用于暂存每行的输出

    if (!(dir = opendir(basePath))) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
        if (stat(path, &fileInfo) == -1) {
            continue;
        }

        // 跳过 "." 和 ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 生成缩进
        line[0] = '\0';  // 清空line
        for (int i = 0; i < depth; i++) {
            strcat(line, "  ");
        }

        if (S_ISDIR(fileInfo.st_mode)) {
            // 目录处理
            sprintf(line + strlen(line), "|-- %s/\n", entry->d_name);
            strcat(content, line);
            listFilesRecursively(path, depth + 1, content);
        } else {
            sprintf(line + strlen(line), "|-- %s\n", entry->d_name);
            strcat(content, line);
        }
    }
    closedir(dir);
}

int tree_dir(int netfd, const char * command_content, const char * src_dir)
{
    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = TREE;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 拼接成完整地址
    char dest_dir[ARR_SIZE] = {0};
    strcat(dest_dir, src_dir);
    strcat(dest_dir, "/");
    strcat(dest_dir, command_content);

    int return_value = 0;
    char content[ARR_SIZE * 10] = {0};
    if(access(dest_dir, F_OK) != 0)
    {// tree目录不存在
        printf("tree目录不存在\n\n");
        return_value = -1;
    }
    else{
        // tree目录存在
        listFilesRecursively(dest_dir, 0, content);
    }

    // 发送响应信息给客户端
    packet.length = strlen(content);
    memset(packet.content, 0, sizeof(packet.content));
    memcpy(packet.content, content, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    if(return_value == -1){
        return -1;
    }
    return 0;
}

int cat_file(int netfd, const char * command_content, const char * src_dir)
{
    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = CAT;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);

    // 拼接成完整地址
    char dest_dir[ARR_SIZE] = {0};
    strcat(dest_dir, src_dir);
    strcat(dest_dir, "/");
    strcat(dest_dir, command_content);

    char content[ARR_SIZE] = {0};
    FILE* fp = fopen(dest_dir, "r");
    if(fp == NULL)
    {// cat文件不存在
        printf("文件不存在\n\n");
        // 发送响应信息给客户端
        packet.length = strlen(content);
        memset(packet.content, 0, sizeof(packet.content));
        memcpy(packet.content, content, packet.length);
        send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        return -1;
    }
    else{
        // cat文件存在
        while(fread(content, sizeof(char), ARR_SIZE, fp) > 0)
        {
            // 发送响应信息给客户端
            packet.length = strlen(content);
            memset(packet.content, 0, sizeof(packet.content));
            memcpy(packet.content, content, packet.length);
            send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
        }
    }
    return 0;
}

int notcmd(int netfd, const char * src_dir)
{
    // 把绝对路径转化为虚拟路径
    char real_dir[ARR_SIZE] = {0};
    getcwd(real_dir, sizeof(real_dir));
    char diff_dir[ARR_SIZE] = {0};
    get_path_diff(real_dir, src_dir, diff_dir);

    // 发送虚拟路径
    tlc_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.type = NOTCMD;
    packet.length = strlen(diff_dir);
    memcpy(packet.content, diff_dir, packet.length);
    send(netfd, &packet, sizeof(packet.type) + sizeof(packet.length) + packet.length, MSG_NOSIGNAL);
    return 0;
}
