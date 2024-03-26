#ifndef __CHAT_SHARE_MEMORY_H__
#define __CHAT_SHARE_MEMORY_H__
//#include <ncurses.h>
typedef struct chatInfo{
    char userID[20];
    long messageTime;
    char message[40];
    int read_flag;
} CHAT_INFO;

#endif//__CHAT_SHARE_MEMORY_H__

