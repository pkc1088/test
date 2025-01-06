#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include <ncurses.h>
#include <pthread.h>
#include "chatshm.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
WINDOW * sw1, *sw2, *sw3, *sw4;
CHAT_INFO *cf = NULL;

void *read_messages(void *arg)                   //유저의 입력 스레드를 관리하는 함수
{        
    char curmsg[40];
    /** for subwin 2 **/
    if(cf->user_index > 2) {mvwprintw(sw4, 9, 9, "too many users"); exit(0);}
    strcpy(cf->userlist[cf->user_index], (char*) arg);  //입력받은 arg를 유저목록에 추가시킵니다
    cf->user_index++;                            //다음 유저를 저장하기 위해 인덱스를 증가시킵니다

    while (1) 
    {
        /** for subwin 3 **/
        echo();                                             
        mvwgetnstr(sw3, 5, 5, curmsg, 40);      //사용자 입력이 서브윈도우3에 보이도록 echo를 설정합니다
        if(strcmp(curmsg, "quit\n") == 0) break;
        wclear(sw3);                            //입력이 끝난 내용은 채팅프로그램 특성상 
        mvwprintw(sw3, 1, 1, "input line\n");   //지워줘야 하기에 clear시킨후 서브윈도우를 다시 그려줍니다
        box(sw3, 0, 0);
        wrefresh(sw3);

        pthread_mutex_lock(&mutex);             //입력받은 메세지를 배열에 넣을 떄 뮤텍스 락을 겁니다
        if(cf->message_index >= 10)             //메세지 10개가 모두 차면
        {
            for(int i = 0; i < 9; i++)          //뒷 원소를 한칸씩 앞으로 당기고
            {
                strcpy(cf->messages[i], cf->messages[i+1]); 
            }                                    //배열의 마지막에 현재 메세지를 저장합니다
            snprintf(cf->messages[9], sizeof(char) * 100,   
                    "   [%s]: %s\n", (char *)arg, curmsg);    
        }   
        else 
        {                   //메세지가 10개 이하이면 messgae_index를 증가시키며 순차적으로 저장합니다
            snprintf(cf->messages[cf->message_index], sizeof(char) * 100, 
                    "   [%s]: %s\n", (char *)arg, curmsg);
            cf->message_index++;
        }
        pthread_mutex_unlock(&mutex);                   //저장 과정이 끝나면 락을 풉니다
    }
    return NULL;
}

void *display_messages(void *arg)              //출력 스레드를 관리하는 함수
{
    /** for subwin1 and subwin2 **/
    while(1) 
    {
        pthread_mutex_lock(&mutex);            //출력시 mutex 락을 겁니다
        for (int i = 0, h = 2; i < cf->message_index; i++, h++) 
        {                                       //서브윈도우1에 메세지 배열을 출력합니다
            mvwprintw(sw1, h, 1, "%s\n", cf->messages[i]);      
        }   
        wrefresh(sw1);
        for (int i = 0, h = 2; i < 3; i++, h++) 
        {                                       //서브윈도우2에 유저 목록을 출력합니다
            if(strcmp(cf->userlist[i],"") == 0) continue;
            mvwprintw(sw2, h, 5, "%s..\n", cf->userlist[i]);   
        }
        wrefresh(sw2);
        usleep(100000);
        pthread_mutex_unlock(&mutex);           //출력이 끝나면 unlock 합니다
    }
    return NULL;
}

int main(int argc, char *argv[]) {  //argv[1]에 유저id를 입력 받습니다.
    initscr();                      //ncurses 초기화 단계입니다.
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    int height, width;
    getmaxyx(stdscr, height, width);
    int sub_height = height / 2;
    int sub_width = width / 2;
    sw1 = newwin(sub_height, sub_width, 0, 0);          //WINDOW *는 전역에 선언되었고 
    sw2 = newwin(sub_height, sub_width, 0, sub_width);  //서브윈도우는 4개로 분할 했습니다.
    sw3 = newwin(sub_height, sub_width, sub_height, 0);
    sw4 = newwin(sub_height, sub_width, sub_height, sub_width);
    box(sw1, 0, 0);                                     //서브윈도우 간 테두리를 생성했습니다.
    box(sw2, 0, 0);
    box(sw3, 0, 0);
    box(sw4, 0, 0);
    mvwprintw(sw1, 1, 1, "Chatting..\n");
    mvwprintw(sw2, 1, 1, "Loged in Users\n");
    mvwprintw(sw3, 1, 1, "input line\n");
    mvwprintw(sw4, 1, 1, "Chat Info\n");
    mvwprintw(sw4, 5, 5, "unlucky kakaotalk");
    
    wrefresh(sw1); wrefresh(sw2); wrefresh(sw3); wrefresh(sw4);   //위 내용들을 반영하기위해 refresh

    pthread_t usrthread, display_thread;
    int shmid; 
    bool init_flag = true;          
	void *shmaddr = (void *)0;
    shmid = shmget((key_t)3936, sizeof(CHAT_INFO), 0666|IPC_CREAT|IPC_EXCL);  //공유메모리를 연결합니다  
    if (shmid < 0)
    {   
        init_flag = false;                      //최초 연결자가 아니면 init_flag를 false로 바꿔줍니다
        shmid = shmget((key_t)3936, sizeof(CHAT_INFO),0666); 
        shmaddr = shmat(shmid, (void *)0, 0666);
        if (shmaddr < 0)
        {   
            perror("shmat attach is failed : ");
            exit(0);
        }
    }

    shmaddr = shmat(shmid, (void *)0, 0666);
    cf = (CHAT_INFO *)shmaddr;                //구조체 cf를 shamaddr과 연결 시킵니다

    if (init_flag)                     //공유메모리를 attach하는 최초의 양수 shmid가
    {                                  //예상치 못한 sig fault를 방지하기 위해 헤더 변수들을 초기화를 시켜줍니다
        cf->user_index = 0; 
        cf->message_index = 0; 
        for (int i = 0; i < 3; i++) strcpy(cf->userlist[i], "");
        for (int i = 0; i < 10; i++) strcpy(cf->messages[i], "");
    }
                                                //각 프로세스는 usrthread로서 메세지를 읽고
    pthread_create(&usrthread, NULL, read_messages, (void *) argv[1]);          
    pthread_create(&display_thread, NULL, display_messages, (void *) argv[1]);  
                                                //display_thread로서 메세지를 출력하도록 멀티스레딩합니다
    /**
    display_messages((void*) argv[1]); 
    스레드 새로 만들 필요없이 이거면 부모 프로세스는 read_message에서
    while 문 안에 있을거고 자식은 display_message 실행해서 while 문 안에 있는 듯
    **/
    pthread_join(usrthread, NULL);
    pthread_join(display_thread, NULL);

    return 0;
}