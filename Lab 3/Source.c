#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define READ 0
#define WRITE 1

int main()
{
	/// ~ File I/O Test Program ~

	//char* binaryPath = "/bin/ls";
	//char* arg1 = "-lh";
	//char* arg2 = "/home";

	//int status;
	//int fd = open("temp.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	//dup2(fd, 1);

 //   pid_t childProcessID = fork();

 //   if (childProcessID < 0) {   // Error
 //       printf("Fork process failed, program exit.\n");
 //       exit(1);
 //   }

 //   if (childProcessID == 0) {  // Child
	//	execl(binaryPath, binaryPath, arg1, arg2, NULL);
 //   }
 //   else {
	//	wait(&status);
	//	printf("Parent reached!\n");
	//	close(fd);
 //   }

	//return 0;

	int fd[2];
	int pipeCreationResult;
	int pid;

	pipeCreationResult = pipe(fd);

	if (pipeCreationResult < 0) {
		perror("Failed pipe creation\n");
		exit(1);
	}

	pid = fork();
	if (pid < 0) // Fork failed
	{
		perror("Fork failed");
		exit(1);
	}

	char myStringOutput[] = "This a test!";
	char myStringInput[50];

	if (pid == 0)
	{ // Child process
		write(fd[1], &myStringOutput, sizeof(myStringOutput));
		printf("Child wrote [%s]\n", myStringOutput);
	}
	else
	{
		read(fd[0], &input, sizeof(int));
		printf("Parent received [%d] from child process\n", input);
	}

	return 0;
}