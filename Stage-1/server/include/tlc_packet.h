#ifndef __TLC_PACKET_H__
#define __TLC_PACKET_H__

#define ARR_SIZE 1024

typedef enum {
    CHATHEAD,
    PWD,
    CD,
    LS,
    LL,
    MKDIR,
    RMDIR,
    RM,
    PUTS,
    GETS,
    TREE,
    CAT,
    NOTCMD,
} cmd_type_t;

typedef struct 
{
    cmd_type_t type; // 命令类型
    int length; //记录命令长度
    char content[ARR_SIZE]; //记录命令内容
} tlc_packet_t;

#endif /* __TLC_PACKET_H__ */
