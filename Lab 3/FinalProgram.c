#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void sigHandler(int sigNum)
{
	switch (sigNum) {
	case SIGINT:
		printf(" received. Time to exit.\n");
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
	int pid;

	int parentPID = getpid();

	printf("-------PROGRAM START--------\n");

	pid = fork();

	if (pid < 0) // Fork failed
	{
		perror("Fork failed");
		exit(1);
	}

	if (pid == 0)
	{ // Child process
		int randomInt = rand() % 10 + 1; // Rand number from 1 - 10
		while (1) {
			sleep(randomInt);
			if ((randomInt % 2) == 0)
				kill(parentPID, SIGUSR1);
			else
				kill(parentPID, SIGUSR2);
			randomInt = rand() % 10 + 1;
		}
	}
	else
	{
		signal(SIGINT, sigHandler);
		signal(SIGUSR1, sigHandler);
		signal(SIGUSR2, sigHandler);

		while (1) {
			printf("waiting... ");
			fflush(stdout);
			pause();
		}
	}

	return 0;
}
