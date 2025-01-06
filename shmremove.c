#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <string.h> 
#include <unistd.h>  
#include "chatshm.h"

int main()
{
	int shmid;

	// 공유메모리 공간 만듦
	shmid = shmget((key_t)3936, sizeof(CHAT_INFO), 0666);

	if (shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	if (shmctl( shmid, IPC_RMID, 0) < 0)
	{
		printf( "Failed to delete shared memory\n");
		return -1;
	}
	else
	{
		printf( "Successfully delete shared memory\n");
	}
	return 0;
}