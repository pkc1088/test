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
	shmid = shmget((key_t)3836, sizeof(CHAT_INFO), 0666|IPC_CREAT|IPC_EXCL);
	if (shmid < 0)
	{
	    shmid = shmget((key_t)3836, sizeof(CHAT_INFO), 0666);
	    shmaddr = shmat(shmid, (void *)0, 0666);
        if (shmaddr == NULL)
	    {
		    perror("shmat attach is failed : ");
		    exit(0);
	    }
    }
    
    shmaddr = shmat(shmid, (void *)0, 0666);
	chatInfo = (CHAT_INFO *)shmaddr;
    shmbufindex = 0;
    readmsgcount = 0; 
    
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
    mvwprintw(subwin4, 5, 5, "%s", "  unlucky kakaotalk");

    int logedUsrNum = 0, messageCnt = 0, msgfull = 0;
    char currentMsg[40];
    char *messageArray[10] = {0, }, *usrlist[3] = {0, };
    
    for(int i = 0; i < 10; i++) 
        messageArray[i] = (char *)malloc(sizeof(char) * 100);
    for(int i = 0; i < 3; i++) 
        usrlist[i] = (char *)malloc(sizeof(char) * 20);


    while(1)
	{   
        /** for SUBWIN 3 **/ 
        wclear(subwin3);
        mvwprintw(subwin3, 1, 1, "input line\n");
        box(subwin3, 0, 0);
        mvwprintw(subwin3, 5, 5, "%s", chatInfo->message);
        wrefresh(subwin3);


        if (1) 
        {
            if(logedUsrNum > 3) {printf("too many users\n"); exit(0);}
            //chatInfo->registered = 1;

            /** for SUBWIN 2 **/   
            wclear(subwin2);
            box(subwin2, 0, 0);
            mvwprintw(subwin2, 1, 1, "Loged in Users\n");

            int found = 0;
            for (int i = 0; i < 3; i++) 
            {
                if (strcmp(usrlist[i], chatInfo->userID) == 0) // 이미 등록
                {   
                    found = 1;
                    break;
                }
            }
            
            if(!found) strcpy(usrlist[logedUsrNum++], chatInfo->userID);
            else goto jump;

            for(int i = 0, h = 5; i < 3; i++, h++) 
            {
                if(usrlist[i] == NULL) break;
                mvwprintw(subwin2, h, 5, "%s\n", usrlist[i]);
            }
            wrefresh(subwin2);
        }
        jump:
        /*
        if (chatInfo->registered == 0) 
        {
            strcpy(usrlist[logedUsrNum++], chatInfo->userID);
            if(logedUsrNum == 3) {printf("too many users\n"); exit(0);}
            chatInfo->registered = 1;

            // for SUBWIN 2   
            wclear(subwin2);
            box(subwin2, 0, 0);
            mvwprintw(subwin2, 1, 1, "Loged in Users\n");

            for(int i = 0, h = 5; i < 3; i++, h++) 
            {
                if(usrlist[i] == NULL) break;
                mvwprintw(subwin2, h, 5, "%s\n", usrlist[i]);
            }
            wrefresh(subwin2);
        }
        */

        if (chatInfo->read_flag == 1)
        {   
	        /*
            snprintf(NewStr, sizeof(NewStr), "   [%s]%ld: %s\n", chatInfo->userID, chatInfo->messageTime, chatInfo->message); 
            mvwscanw(subwin1,0,0,NewStr);
            for( i=0; i<nheight-1; i++ )
                strcpy(ChatStr[i],ChatStr[i+1]);
            strcpy(ChatStr[nheight-1],NewStr);
            for( i=0; i<nheight-1; i++ )
            mvwprintw(subwin1,i+1,1,ChatStr[i]);
            */
            if(msgfull == 0 && messageCnt < 9) messageCnt++;
            else {msgfull = 1; messageCnt = 9;}

            if(msgfull == 0)
                snprintf(messageArray[messageCnt], sizeof(char *) * 100, 
                        "   [%s]%ld: %s\n", chatInfo->userID, 
                        chatInfo->messageTime, chatInfo->message); 

            if(msgfull == 1)
            {
                for(int i = 0; i < 9; i++) 
                    strcpy(messageArray[i], messageArray[i + 1]);
                    
                snprintf(messageArray[9], sizeof(char *) * 100, 
                        "   [%s]%ld: %s\n", chatInfo->userID, 
                        chatInfo->messageTime, chatInfo->message); 
            } 

            wclear(subwin1);
            box(subwin1, 0, 0);
            mvwprintw(subwin1, 1, 1, "Chatting..\n");
            for(int i = 0, h = 2; i < 10; i++, h++)
            {
                if(messageArray[i]==NULL) break;
                mvwprintw(subwin1, h, 1, "%s\n", messageArray[i]);
                wrefresh(subwin1);
            }

            chatInfo->read_flag = 0;
        }

        wrefresh(subwin4);
                
        if (!strcmp(chatInfo->message, "/exit\n")) 
        {
            printf("%s is out\n", chatInfo->userID);
            break; 
        }
	    sleep(1);
	}
}