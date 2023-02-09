/////////////////////////////
//David Geisel & Seth Gibson
/////////////////////////////

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

//DG - shamlessly stolen from:
// https://stackoverflow.com/questions/71274207/how-to-bold-text-in-c-program
#define COLOR_BOLD "\e[1m"
#define COLOR_OFF "\e[m"
//

#define READ 0
#define WRITE 1
#define MSGSIZE 256
bool INPUTFLAG;

//TODO
//Signal handlers don't close pipes - im not 100% sure we need to do this but idk
//When I tested 255 nodes the fgets wouldn't prompt for input, it'd just go with "\n"
//Inputting 1 to send the message to node 0 is confusing, in my opinion, not a huge deal tho
//

void sigHandlerParent(int sigNum){
    switch(sigNum){
        case SIGINT:
            //ctrl c
            //TODO close pipes
            exit(0);
    }
    
}

//Resets the input flag so parent process takes another input
//DG - This wasn't working when combined with the other sighandler idk why but it works now
void takeNewInput(){
	printf("[Parent] USERINPUT reset.\n");
	INPUTFLAG = false;
}

void sigHandlerChild(int sigNum){
    switch(sigNum){
        case SIGINT:
            //ctrl c
            //TODO close pipes
            exit(0);
    }
}

int main(){

	INPUTFLAG = false;
	int numChildren;
    int pid, currentChild;
	int parentPID = getpid();

	//Take input for how many nodes to create
	char numInput[1];
	do {
		printf("Number of nodes [1-255]: ");
		fgets(numInput, sizeof(numChildren), stdin);
		numChildren = atoi(numInput);
	}
	while(numChildren < 1);

	//Init pipes
    int fd[numChildren + 1][2];
    for (int i = 0; i < numChildren; i++) {
        if (pipe(fd[i]) < 0) {
            printf("Error in pipe creation. Exiting...\n");
            exit(1);
        }
    }

	//Init nodes
	for (currentChild = 0; currentChild < numChildren; currentChild++) {
        pid = fork();
        if (pid < 0) // Fork failed
        {
            perror("Fork failed");
            exit(1);
        }
        if (pid == 0)
            break;
    }

	if(pid == 0){ // Children processes

		signal(SIGINT, sigHandlerParent);

		// Close all not-in-use pipe openings
        for (int i = 0; i < numChildren + 1; i++) {
            for (int j = 0; j < 2; j++) {
                if (i - currentChild != j) {
                    close(fd[i][j]);
                }
            }
        }

		printf("[Node #%d] Extra pipes closed, entering read loop.\n", currentChild);

		//Child process loop
		char apple[256];
		while(1){
			printf("[Node #%d] Waiting for apple at [%d][0]\n", currentChild, currentChild);

    		//wait until pipe contains a message
    		if(read(fd[currentChild][0], &apple, MSGSIZE) < 0){
				printf("Error in Child #%d read. Exiting...\n", currentChild);
            	exit(1);
			}
			
    		//Deconstruct apple
    		char *token;
    		char dest[16];
    		char msg[256];

			//DG - had to add this because strtok was modifying apple & somehow preventing the next node from reading properly
			char appleFull[256]; 
			strcpy(appleFull, apple);

    		token = strtok(apple, ":");
    		strcpy(dest, token);
    		token = strtok(NULL, ":");
    		strcpy(msg, token);

    		printf("[Node #%d] Got apple from [%d][%d]. Message is for node #%d.\n", currentChild, currentChild, 0, atoi(dest));

    		//Check if message is for this node
    		if(atoi(dest) == currentChild){
        		printf(COLOR_BOLD "[Node #%d] got message: %s\n" COLOR_OFF, currentChild, msg); //bold for visibility
        		kill(parentPID, SIGUSR1);
        		continue;

    		}else{
    		    printf("[Node #%d] Writes apple to [%d][1].\n", currentChild, currentChild + 1);
    		    write(fd[currentChild + 1][1], appleFull, MSGSIZE);
    		    continue;
    		}

		}

	}else{

		//Parent Process loop
		signal(SIGINT, sigHandlerParent);
		signal(SIGUSR1, takeNewInput);
		while(1){

		    INPUTFLAG = true;
		    char message[256];
		    char dest[16];

			sleep(1);//just for convenience, otherwise gets buried in debug msgs
		    printf("Message [Max 256 Characters]: ");
		    fgets(message, sizeof(message), stdin);
			strtok(message, "\n"); //trim newline char, was causing problems
			printf("[Parent] Reading message \"%s\"\n", message);

		    printf("Destination [%d-%d]: ", 1, numChildren);
		    fgets(dest, sizeof(dest), stdin); 
			int tempInt = atoi(dest);
		    if(tempInt < 1 || tempInt > numChildren){
		        printf("\nInvalid Destination.\n");
		        continue;
		    }

			sprintf(dest, "%d", tempInt - 1); //decrements string
		    strcat(dest, ":");
		    strcat(dest, message);
			printf("[Parent] Sending apple: \"%s\"\n", dest);
		    write(fd[0][1], dest, MSGSIZE);
			printf("[Parent] Waiting for INPUTFLAG.\n");
		    while(INPUTFLAG);
		}
	}

	return 0;
}
