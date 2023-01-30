#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// DISCLAIMER!!!!
// Okay, this is a mess, but only because this is two hours worth of very distracted work.
// The program can create any amount of children from the same parent, but the pipes used
// to get them talking to each other haven't been finished yet. There's a lot of fat that
// needs to be trimmed, but only because I hate deleting shit I might need later. 
// This is VERY EARLY and needs a lot of work. We can beautify it later.

int main()
{
	int userInputNodes = 5;
	int pid, status, i;

	int fd[userInputNodes * 2];
	int pipeCreationResult;

	pipeCreationResult = pipe(fd);

	if (pipeCreationResult < 0) {
		perror("Failed pipe creation\n");
		exit(1);
	}

	for (i = 1; i <= userInputNodes; i++) {
		pid = fork();
		if (pid < 0) // Fork failed
		{
			perror("Fork failed");
			exit(1);
		}
		if (pid == 0)
			break;
	}

	if (pid == 0) { // Child process
		printf("[%d] - This process is Child #%d!\n", getpid(), i);
	}
	else
	{
		for (int j = 0; j < userInputNodes; j++) {
			wait(&status);
		}

		printf("[%d] - This process is the parent!\n", getpid());
	}

	return 0;
}
