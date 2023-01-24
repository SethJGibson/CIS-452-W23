#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {

    char* input = malloc(sizeof(char) * 64);
    pid_t childProcessID;

    printf("Input: ");
    fgets(input, sizeof(input), stdin);
    printf("your input: %s\n", input);
    //TODO quit

    char* command = malloc(sizeof(char) * 64);
    char* args = malloc(sizeof(char) * 64);
    command = strsep(&input, " ");
    args = strsep(&input, "\n");
    printf("Command: %s  Args: \"%s\"\n", command, args);

    if ((childProcessID = fork()) < 0) {
        printf("Fork Failed");
        exit(1);
    }
    if (childProcessID == 0) {
        if (execlp(command, command, args, (char*)0) < 0) {
            perror("exec failed");
            exit(1);
        }
    }
    else {
        wait(-1);
        printf("Parent done");
        exit(0);
        //resource usage stats
    }
}
