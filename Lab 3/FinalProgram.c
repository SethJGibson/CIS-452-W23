#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

//#define READ 0
//#define WRITE 1

void sigHandler(int sigNum)		// Initially thought the freezing issue was due to multiple handlers. I'll undo when the freezing issue is resolved.
{
	switch (sigNum) {
	case SIGINT:
		printf("^C received. Time to exit.\n");
		exit(0);
		break;
	case SIGUSR1:
		printf("Received SIGUSR1 Signal...\n");
		break;
	case SIGUSR2:
		printf("Received SIGUSR2 Signal...\n");
		break;
	}
}

int main()
{
	int fd[2];
	//int pipeCreationResult;
	int pid;

	//pipeCreationResult = pipe(fd);

	//if (pipeCreationResult < 0) {
	//	perror("Failed pipe creation\n");
	//	exit(1);
	//}

//	signal(SIGINT, sigHandler);
//	signal(SIGUSR1, sigHandler);
//	signal(SIGUSR2, sigHandler);

	printf("-------PROGRAM START--------\n");

	pid = fork();

	if (pid < 0) // Fork failed
	{
		perror("Fork failed");
		exit(1);
	}

	//int output = 3;
	//int input;

	if (pid == 0)
	{ // Child process
		//write(fd[1], &output, sizeof(int));
		//printf("Child wrote [%d]\n", output);

		int randomInt = rand() % 10 + 1; // Rand number from 1 - 10
		while (1) {
			sleep(randomInt);
			if ((randomInt % 2) == 0)
				kill(pid, SIGUSR1);
			else
				kill(pid, SIGUSR2);
			randomInt = rand() % 10 + 1;
		}
	}
	else
	{
		//read(fd[0], &input, sizeof(int));
		//printf("Parent received [%d] from child process\n", input);

		signal(SIGINT, sigHandler);
		signal(SIGUSR1, sigHandler);
		signal(SIGUSR2, sigHandler);

		while (1) {
			printf("waiting...\t");
			pause();
		}
	}

	return 0;
}
