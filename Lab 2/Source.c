#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static const unsigned int BUFFER_SIZE = 64;

int main(int argc, char* argv[]) {
    // Gonna try something from https://stackoverflow.com/questions/37273193/tokenize-user-input-for-execvp-in-c

    char input[64];
    int status;

    struct rusage ru;

    printf("You have shell!\n");

    while (1) {
        printf("\nuser@gvsu> ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;    // Didn't copy this from above source, went out and double-checked it: https://stackoverflow.com/questions/2693776/removing-trailing-newline-character-from-fgets-input

        if (strncmp(input, "quit", 4) == 0) {
            exit(1);
        }

        char* token = strtok(input, " ");
        char* args[64];
        int argCounter = 0;

        while (token != NULL) {
            args[argCounter] = token;
            argCounter++;
            token = strtok(NULL, " ");
        }
        args[argCounter] = NULL;

        pid_t childProcessID = fork();

        if (childProcessID < 0) {   // Error
            printf("Fork process failed, program exit.\n");
            exit(1);
        }

        if (childProcessID == 0) {  // Child
            execvp(args[0], args);
        }
        else {
            wait(&status);

            if (getrusage(RUSAGE_CHILDREN, &ru) == -1) {
                printf("rusage failed, program exit.\n");
                exit(1);
            }

            printf("\nInvoluntary context switches: %ld\n", ru.ru_nivcsw);
            printf("User CPU Usage Time (us):     %ld\n", ru.ru_utime.tv_usec);
        }
    }
}


