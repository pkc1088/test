#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include <ncurses.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include "chatshm.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t usrthread, display_thread;    // 스레드는 전역으로 배치
WINDOW * sw1, *sw2, *sw3, *sw4;         // 서브윈도우
CHAT_INFO *cf = NULL;                   // 공유메모리에 연결될 구조체

sem_t *sem;                             // 세마포어
char cname[10];                         // argv[1] 값을 저장할 배열

void init_screen(); // 초기 화면 구성 프로토타입

void *read_messages(void *arg)                 
{   
    // 시그널 핸들러 함수에서 스레드 캔슬 발생시 즉각 처리
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);   
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
             
    char curmsg[40];
    //
    
    //char secret_msg[10][40];

    
    //
    /** for subwin 2 **/
    // 중복된 이름 있다면 종료시킴
    for (int i = 0; i < 3; i++) 
        if(strcmp(cf->userlist[i], (char*)arg) == 0) {endwin(); exit(0);}

    // 유저수가 3명인데 입장한 경우 unlink후 종료
    if(cf->user_no > 2) {sem_unlink((char*)arg); endwin(); exit(0);}

    // 유저 목록의 빈 곳에 새로운 유저 추가
    for (int i = 0; i < 3; i++) 
    {
        if(strcmp(cf->userlist[i], "") == 0) 
        {
            strcpy(cf->userlist[i], (char*) arg); 
            cf->user_no++;   
            break;                         
        } 
    }

    while(1) 
    {
        /** for subwin 3 **/
        echo();                                             
        mvwgetnstr(sw3, 5, 5, curmsg, 40);      
        wclear(sw3);                            
        mvwprintw(sw3, 1, 1, "input line\n");   
        box(sw3, 0, 0);
        wrefresh(sw3);

        sem_wait(sem);  // sem 값이 1일때 감소시키고 다음 코드 실행
        
        // 메세지가 10개 초과될 시 한 칸씩 앞으로 배치 후 새 메세지 추가
        if(cf->message_index >= 10)             
        {
            for (int i = 0; i < 9; i++)         
            {
                strcpy(cf->messages[i], cf->messages[i+1]); 
            }                                    
            snprintf(cf->messages[9], sizeof(char) * 60,   
                    "[%s]: %s", (char *)arg, curmsg);    
        }   
        else 
        {                  
            snprintf(cf->messages[cf->message_index], sizeof(char) * 60, 
                    "[%s]: %s", (char *)arg, curmsg);
            cf->message_index++;
        }
        sem_post(sem);  // sem 값을 1 증가시켜 동기화
    }

    return NULL;
}

void *display_messages(void *arg)              
{
    // 시그널 핸들러 함수에서 스레드 캔슬 발생시 즉각 처리
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    /** for subwin1 and subwin2 **/
    while(1) 
    {
        sem_wait(sem);  // sem 값을 1 감소시킴
        
        wclear(sw1);
        box(sw1, 0, 0);
        mvwprintw(sw1, 1, 1, "Chatting..");
        for (int i = 0, h = 2; i < cf->message_index; i++, h++) 
        {                                      
            mvwprintw(sw1, h, 3, "%s", cf->messages[i]);        
        }   
        wrefresh(sw1);

        wclear(sw2);
        mvwprintw(sw2, 1, 1, "Loged in Users\n");
        box(sw2, 0, 0);
        // 로그인된 유저 목록 출력
        for (int i = 0, h = 2; i < 3; i++, h++) 
        {                                       
            if(strcmp(cf->userlist[i],"") == 0) {h--; continue;}
            mvwprintw(sw2, h, 5, "%s..", cf->userlist[i]);   
        }
        wrefresh(sw2);

        sem_post(sem);  // sem 값을 1 증가시켜 동기화
        usleep(50000);  // 화면 반짝임 없앰
    }

    return NULL;
}

void sigint_handler(int signum) {       

    // 컨트롤 c 입력시 실행 중인 스레드에게 취소요청 보냄
    pthread_cancel(usrthread);  
    pthread_cancel(display_thread);
    cf->user_no--;  // 유저 1명 감소
    // 후에 입장할 유저를 위해 퇴장한 유저 자리를 빈칸으로 둠     
    for (int i = 0; i < 3; i++) 
    {                                       
        if(strcmp(cf->userlist[i], cname) == 0) 
        {
            strcpy(cf->userlist[i], "");
            break;
        }
    }
}


int main(int argc, char *argv[]) {  

    init_screen();
    // argv[1]값을 cname에 복사시켜 시그널핸들러 함수가 cname을 사용하게 함
    strcpy(cname, argv[1]);
    // argv[1]의 이름을 갖는 네임드 세마포어, 초기 sem 값을 1로 줌
    sem = sem_open(argv[1], O_CREAT, 0666, 1);
    // 시그널 핸들러 함수 등록
    signal(SIGINT, sigint_handler);
    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    int shmid; 
    bool init_flag = true;          
	void *shmaddr = (void *)0;
    // 헤더파일의 구조체와 공유 메모리를 연결
    shmid = shmget((key_t)3936, sizeof(CHAT_INFO), 0666|IPC_CREAT|IPC_EXCL); 
    if (shmid < 0)
    {   
        init_flag = false;             
        shmid = shmget((key_t)3936, sizeof(CHAT_INFO),0666); 
        shmaddr = shmat(shmid, (void *)0, 0666);
        if (shmaddr < 0)
        {   
            perror("shmat attach is failed : ");
            exit(0);
        }
    }

    shmaddr = shmat(shmid, (void *)0, 0666);
    cf = (CHAT_INFO *)shmaddr;               
    
    if (init_flag)                   
    {                                 
        cf->user_no = 0; 
        cf->message_index = 0; 
        for (int i = 0; i < 3; i++) strcpy(cf->userlist[i], "");
        for (int i = 0; i < 10; i++) strcpy(cf->messages[i], "");
    }
    
    // 메세지를 읽는 스레드와, 메세지를 화면에 출력하는 스레드 생성
    pthread_create(&usrthread, NULL, read_messages, (void *) argv[1]);          
    pthread_create(&display_thread, NULL, display_messages, (void *) argv[1]);  
                                             
    // 메인 함수는 스레드들을 기다려 먼저 종료되지 않게 함
    pthread_join(usrthread, NULL);
    pthread_join(display_thread, NULL);

    // 시그널 발생 후 스레드들이 종료되면 sem을 unlink시킴
    sem_unlink(argv[1]);
    sem_close(sem);
    // 프로세스 내 유저수가 0명이 되면 공유메모리 해제를 진행
    if(cf->user_no == 0) system("./shmremove");
    endwin();   

    return 0;
}

void init_screen() {
    // 서브윈도우 4개로 분할 후 테두리 생성
    initscr();                     
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    int height, width;
    getmaxyx(stdscr, height, width);
    int sub_height = height / 2;
    int sub_width = width / 2;
    sw1 = newwin(sub_height, sub_width, 0, 0);          
    sw2 = newwin(sub_height, sub_width, 0, sub_width);  
    sw3 = newwin(sub_height, sub_width, sub_height, 0);
    sw4 = newwin(sub_height, sub_width, sub_height, sub_width);
    box(sw1, 0, 0);                                     
    box(sw2, 0, 0);
    box(sw3, 0, 0);
    box(sw4,0, 0);
    mvwprintw(sw1, 1, 1, "Chatting..");
    mvwprintw(sw2, 1, 1, "Loged in Users");
    mvwprintw(sw3, 1, 1, "input line");
    mvwprintw(sw4, 1, 1, "Chat Info");
    mvwprintw(sw4, 5, 5, "unlucky kakaotalk");
    
    wrefresh(sw1); wrefresh(sw2);wrefresh(sw3);wrefresh(sw4);  
}
