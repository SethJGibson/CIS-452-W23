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

//DG - shamelessly stolen from:
// https://stackoverflow.com/questions/71274207/how-to-bold-text-in-c-program
#define COLOR_BOLD "\e[1m"
#define COLOR_OFF "\e[m"
#define READ 0
#define WRITE 1
#define MSGSIZE 256

bool INPUTFLAG;

//TODO
//Signal handlers don't close pipes - im not 100% sure we need to do this but idk
//

void sigHandlerParent(int sigNum) {
	switch (sigNum) {
	case SIGINT:
		//printf("\nParent received shutdown signal. Exiting...");
		exit(0);
	default:
		printf("Error in SIGINT handling. Exiting...\n");
		exit(1);
	}
}

void sigHandlerChild(int sigNum) {
	switch (sigNum) {
	case SIGINT:
		//printf("\nChild received shutdown signal. Exiting...");
		exit(0);
	default:
		printf("Error in SIGINT handling. Exiting...\n");
		exit(1);
	}
}

//Resets the input flag so parent process takes another input
//DG - This wasn't working when combined with the other sighandler idk why but it works now
void takeNewInput() {
	printf("[Parent] USERINPUT reset.\n");
	INPUTFLAG = false;
}

int main() {

	INPUTFLAG = false;
	int numChildren;
	int pid, currentChild;
	int parentPID = getpid();

	printf("\nHotApple v1.0.0 Online!\n");

	//Take input for how many nodes to create
	char numInput[1];
	do {
		printf("Number of processes to spawn [1-255]: ");
		fgets(numInput, sizeof(numChildren) + 1, stdin);
		numChildren = atoi(numInput);
	} while (numChildren < 1);

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

	if (pid == 0) { // Children processes

		signal(SIGINT, sigHandlerChild);

		printf("[Child #%d] Process spawned!\n", currentChild + 1);

		// Close all not-in-use pipe openings
		for (int i = 0; i < numChildren + 1; i++) {
			for (int j = 0; j < 2; j++) {
				if (i - currentChild != j) {
					close(fd[i][j]);
				}
			}
		}

		printf("[Child #%d] Extra pipes closed, entering read loop.\n", currentChild + 1);

		//Child process loop
		char apple[256];
		while (1) {
			printf("[Child #%d] Waiting for Apple at [%d][0]...\n", currentChild + 1, currentChild);

			//wait until pipe contains a message
			if (read(fd[currentChild][0], &apple, MSGSIZE) < 0) {
				printf("Error in Child #%d read. Exiting...\n", currentChild + 1);
				exit(1);
			}

			//Deconstruct apple
			char* token;
			char dest[16];
			char msg[256];

			//DG - had to add this because strtok was modifying apple & somehow preventing the next node from reading properly
			char appleFull[256];
			strcpy(appleFull, apple);

			token = strtok(apple, ":");
			strcpy(dest, token);
			token = strtok(NULL, ":");
			strcpy(msg, token);

			printf("[Child #%d] Got Apple from [%d][%d]. Message is for Child #%d.\n", currentChild + 1, currentChild + 1, 0, atoi(dest) + 1);

			//Check if message is for this node
			if (atoi(dest) == currentChild) {
				printf(COLOR_BOLD "[Child #%d] Apple reached destination!: %s\n" COLOR_OFF, currentChild + 1, msg); //bold for visibility
				kill(parentPID, SIGUSR1);
				continue;
			}
			else {
				printf("[Child #%d] Writing Apple to [%d][1]...\n", currentChild + 1, currentChild + 1);

				if (write(fd[currentChild + 1][1], appleFull, MSGSIZE) < 0) {
					printf("Error in Child #%d write. Exiting...\n", currentChild + 1);
					exit(1);
				}

				continue;
			}

		}

	}
	else {	// PARENT

		//Parent Process loop
		signal(SIGINT, sigHandlerParent);
		signal(SIGUSR1, takeNewInput);
		while (1) {

			INPUTFLAG = true;
			char message[256];
			char dest[16];

			sleep(1);												// Included for convenience, otherwise parent messages get buried in child messages

			printf("\nMessage [Max 256 Characters]: ");
			fgets(message, sizeof(message), stdin);
			strtok(message, "\n");									// Trim newline char, was causing problems
			printf("[Parent] Reading message \"%s\"\n", message);

			int tempInt;
			do {
				printf("Destination [%d-%d]: ", 1, numChildren);
				fgets(dest, sizeof(dest), stdin);
				tempInt = atoi(dest);

				if (tempInt < 1 || tempInt > numChildren) {
					printf("Invalid Destination. Please try again.\n");
				}
			} while (tempInt < 1 || tempInt > numChildren);

			sprintf(dest, "%d", tempInt - 1);						// Decrement value for operation of pipe system
			strcat(dest, ":");
			strcat(dest, message);

			printf("\n[Parent] Sending apple: \"%s\"...", dest);
			if (write(fd[0][1], dest, MSGSIZE) < 0) {
				printf("Error in Parent #%d read. Exiting...\n", currentChild + 1);
				exit(1);
			}
			printf(" Sent!\n");

			printf("[Parent] Waiting for INPUTFLAG.\n");

			while (INPUTFLAG);										// Wait until apple has reached destination
		}
	}

	return 0;
}
