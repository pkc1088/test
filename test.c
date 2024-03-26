#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

	char *messageArray[10] = {0, };
	for(int i = 0; i < 10; i++) 
        messageArray[i] = (char *)malloc(sizeof(char) * 100);

	char inputstr[40];
    printf("Enter your message: ");
    fgets(inputstr, 40, stdin); 
	strcpy(messageArray[0], inputstr); 

	printf("%s\n", messageArray[0]);
	

    return 0;
}
