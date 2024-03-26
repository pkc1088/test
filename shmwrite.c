#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h> 
#include "chatshm.h"

//사용자의 write를 읽어들임

int main(int argc, char* argv[])
{
	int shmid;
    int shmbufindex;
    int msgindex;
    int totalmessage;
    char userID[20];
 
	CHAT_INFO *chatinfo= NULL;
	void *shmaddr = (void *)0;
    
    if (argc < 2) {
        fprintf(stderr, "[Usage]: ./shmwrite UserID \n");
        exit(0);
    }
    
    strcpy(userID, argv[1]);   
    shmid = shmget((key_t)3836, sizeof(CHAT_INFO),
                                0666|IPC_CREAT|IPC_EXCL);
    
    if (shmid < 0)
    {   
        shmid = shmget((key_t)3836, sizeof(CHAT_INFO),0666); 
        shmaddr = shmat(shmid, (void *)0, 0666);
        if (shmaddr < 0)
        {   
            perror("shmat attach is failed : ");
            exit(0);
        }
    }

    shmaddr = shmat(shmid, (void *)0, 0666);
    chatinfo = (CHAT_INFO *)shmaddr;
	
    while(1)
	{
        char inputstr[40];
        printf("Enter your message: ");
        fgets(inputstr, 40, stdin); 
        
        chatinfo->read_flag = 1;
        
        strcpy(chatinfo->userID, userID);
        chatinfo->messageTime++;
        strcpy(chatinfo->message, inputstr);  

        //
        //chatinfo->registered = 0;
	}
}