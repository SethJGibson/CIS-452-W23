#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

// Using method from https://code-vault.net/course/46qpfr4tkz:1603732431896/lesson/mrvonl5pmz:1603732432986

int proof(void) {
	int userInputChildren = 5;
	int pid, currentChild;

    int fd[userInputChildren + 1][2];
    for (int i = 0; i < userInputChildren; i++) {
        if (pipe(fd[i]) < 0) {
            printf("Error in pipe creation. Exiting...\n");
            exit(1);
        }
    }

    for (currentChild = 0; currentChild < userInputChildren; currentChild++) {
        pid = fork();
        if (pid < 0) // Fork failed
        {
            perror("Fork failed");
            exit(1);
        }
        if (pid == 0)
            break;
    }

    if (pid == 0) { // CHILD
        // Close all not-in-use pipe openings
        for (int i = 0; i < userInputChildren + 1; i++) {
            for (int j = 0; j < 2; j++) {
                if (i - currentChild != j) {
                    close(fd[i][j]);
                }
            }
        }

        int hotPotata = 0;

        if (read(fd[currentChild][0], &hotPotata, sizeof(int)) < 0) {
            printf("Error in Child #%d read. Exiting...\n", currentChild);
            exit(1);
        }
        printf("Child #%d received %d!\n", currentChild, hotPotata);
        if (write(fd[currentChild + 1][1], &hotPotata, sizeof(int)) < 0) {
            printf("Error in Child #%d write. Exiting...\n", currentChild);
            exit(1);
        }

        close(fd[currentChild][0]);
        close(fd[currentChild + 1][1]);

        return;
    }

    // PARENT
    for (int i = 0; i < userInputChildren + 1; i++) {
        for (int j = 0; j < 2; j++) {
            if ((i == userInputChildren && j == 0) || (i == 0 && j == 1));
            else
                close(fd[i][j]);
        }
    }
    
    int hotPotata = 42;

    if (write(fd[0][1], &hotPotata, sizeof(int)) < 0) {
        printf("Error in Parent write. Exiting...\n");
        exit(1);
    }
    if (read(fd[userInputChildren][0], &hotPotata, sizeof(int)) < 0) {
        printf("Error in Parent read. Exiting...\n");
        exit(1);
    }
    printf("Result is %d\n", hotPotata);
    close(fd[0][1]);
    close(fd[userInputChildren][0]);

    //waitpid(pid1, NULL, 0);   // Need to store the pid of each child
    //waitpid(pid2, NULL, 0);   // in order to do clean shutdown here

	return 0;
}