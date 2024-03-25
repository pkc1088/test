#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include <ncurses.h>
#include "chatshm.h"

//read 해서 화면에 출력

int main()
{
    int shmid;
	int shmbufindex, readmsgcount;
	CHAT_INFO *chatInfo= NULL;
	void *shmaddr = (void *)0;
	shmid = shmget((key_t)3836, sizeof(CHAT_INFO), 
                        0666|IPC_CREAT|IPC_EXCL);
	if (shmid < 0)
	{
	    shmid = shmget((key_t)3836, sizeof(CHAT_INFO), 0666);
	    shmaddr = shmat(shmid, (void *)0, 0666);
        if (shmaddr < 0)
	    {
		    perror("shmat attach is failed : ");
		    exit(0);
	    }
    }
    shmaddr = shmat(shmid, (void *)0, 0666);
	chatInfo = (CHAT_INFO *)shmaddr;
    shmbufindex = 0;
    readmsgcount = 0; 
    
    //
    //strucArray[3];
    CHAT_INFO* strucArray[3] = {0, };
    for(int i = 0; i < 3; i++) 
    {
        strucArray[i] = (CHAT_INFO *)malloc(sizeof(CHAT_INFO)); 
    }
    char messageArray[10];
    //
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    int height, width;
    getmaxyx(stdscr, height, width);
    int sub_height = height / 2;
    int sub_width = width / 2;
    WINDOW *subwin1 = newwin(sub_height, sub_width, 
                             0, 0);
    WINDOW *subwin2 = newwin(sub_height, sub_width, 
                             0, sub_width);
    WINDOW *subwin3 = newwin(sub_height, sub_width, 
                             sub_height, 0);
    WINDOW *subwin4 = newwin(sub_height, sub_width, 
                             sub_height, sub_width);
    box(subwin1, 0, 0);
    box(subwin2, 0, 0);
    box(subwin3, 0, 0);
    box(subwin4, 0, 0);
    mvwprintw(subwin1, 1, 1, "Chatting..\n");
    mvwprintw(subwin2, 1, 1, "Loged in Users\n");
    mvwprintw(subwin3, 1, 1, "input line\n");
    mvwprintw(subwin4, 1, 1, "Chat Info\n");
    wprintw(subwin4, "%s", "  unlucky kakaotalk");


    char *logedUsr[3] = {NULL};
    int logedUsrNum = 0;

/*

            CHAT_INFO에 대해 구조체 배열을 만들어야 됨!
            = chatinfo임 / strucArray
            -> 그 배열에서 usrID를 subwin2에 출력해야함
*/

	while(1)
	{   
        wclear(subwin3);
        mvwprintw(subwin3, 1, 1, "input line\n");
        box(subwin3, 0, 0);
        mvwprintw(subwin3, 5, 5, "%s", chatInfo->message);
        
        if (chatInfo->registered == 0) {
            logedUsr[logedUsrNum++] = chatInfo->userID;
            //이게 아니라 chatinfo에서 registered 값 확인 해서 반영
            //strucArray
            chatInfo->registered = 1;
        }
        
        if (chatInfo->read_flag == 1)
        {   
            //messageArray에 메세지 저장 훟 subwin1에 출력
	        wprintw(subwin1, "   [%s]%ld: %s\n", 
            chatInfo->userID,
            chatInfo->messageTime, 
            chatInfo->message);
            
            wrefresh(subwin1);
            wrefresh(subwin2);
            wrefresh(subwin3);
            wrefresh(subwin4);
            chatInfo->read_flag = 0;
        }
        
        wclear(subwin2);
        box(subwin2, 0, 0);
        mvwprintw(subwin2, 1, 1, "Loged in Users\n");
        
        for(int i = 0; i < 3; i ++) 
        {
            //if(logedUsr[i] == NULL) break;
            wprintw(subwin2, "%s\n", logedUsr[i]);
            //이게 아니라 chatinfo에서 registered 값 확인 해서 반영


        }
        

        if (!strcmp(chatInfo->message, "/exit\n")) 
        {
            printf("%s is out\n", chatInfo->userID);
            break; 
        }
	    sleep(1);
	}
    
}