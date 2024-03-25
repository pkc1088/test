#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <ncurses.h>
#include "chatshm.h"
int main() {
    // 공유 메모리 키 생성
    key_t key = ftok("shmfile",65);

    // 공유 메모리에 연결
    int shmid = shmget(key,sizeof(CHAT_INFO),0666|IPC_CREAT);
    CHAT_INFO *chatInfo = (CHAT_INFO*) shmat(shmid,(void*)0,0);

/*
    // ncurses 초기화
    initscr();
    cbreak();
    noecho();
*/  
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    // 윈도우 크기 가져오기
    int height, width;
    getmaxyx(stdscr, height, width);

    // 각 서브 윈도우의 크기 계산
    int sub_height = height / 2;
    int sub_width = width / 2;

    // 2x2 그리드에 4개의 서브 윈도우 생성
    WINDOW *subwin1 = newwin(sub_height, sub_width, 0, 0);
    WINDOW *subwin2 = newwin(sub_height, sub_width, 0, sub_width);
    WINDOW *subwin3 = newwin(sub_height, sub_width, 
                             sub_height, 0);
    WINDOW *subwin4 = newwin(sub_height, sub_width, 
                             sub_height, sub_width);

    // 각 서브 윈도우에 테두리 추가
    box(subwin1, 0, 0);
    box(subwin2, 0, 0);
    box(subwin3, 0, 0);
    box(subwin4, 0, 0);

    // 각 서브 윈도우에 내용 작성
    mvwprintw(subwin1, 1, 1, "Window 1");
    mvwprintw(subwin2, 1, 1, "Window 2");
    mvwprintw(subwin3, 1, 1, "Window 3");
    mvwprintw(subwin4, 1, 1, "Window 4");

    //printw("[%s]%ld: %s\n", chatInfo->userID, 
    //        chatInfo->messageTime, chatInfo->message);
    
    // 각 서브 윈도우를 색칠
    wbkgd(subwin1, COLOR_PAIR(1));
    wbkgd(subwin2, COLOR_PAIR(2));
    wbkgd(subwin3, COLOR_PAIR(3));
    wbkgd(subwin4, COLOR_PAIR(4));

    while(1) 
    {
        // 화면 업데이트
        refresh();
        // 각 서브 윈도우 업데이트
        wrefresh(subwin1);
        wrefresh(subwin2);
        wrefresh(subwin3);
        wrefresh(subwin4);
        // 유저 입력 대기
        getch();
        // ncurses 종료
        endwin();
        
        sleep(3);
    }


    return 0;
}
