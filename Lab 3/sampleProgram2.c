#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define READ 0
#define WRITE 1

int main(){
    int fd[2];
    int pipeCreationResult;
    int pid;
    pipeCreationResult = pipe(fd);
    if(pipeCreationResult < 0){
        perror("Failed pipe creation\n");
        exit(1);
    }
    pid = fork();
    if(pid < 0) // Fork failed
    {
        perror("Fork failed");
        exit(1);
    }

    char myStringOutput[] = "This a test!";
    char myStringInput[50];
    int output = 3;
    int input;

    if(pid == 0)
    { // Child process
        write(fd[1], &myStringOutput, sizeof(myStringOutput));
        printf("Child wrote [%s]\n", myStringOutput);
    }else{
        int bytesRead = read(fd[0], &myStringInput, sizeof(myStringInput));
        printf("Parent received [%s] from child process and read [%d] bytes.\n", myStringInput, bytesRead);
    }

    return 0;
}