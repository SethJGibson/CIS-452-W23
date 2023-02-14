#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>

int SHUTDOWNFLAG = 0;

void sigHandlerWriter(int sigNum) {
    switch (sigNum) {
    case SIGINT:
        SHUTDOWNFLAG = 1;
        exit(0);
    default:
        printf("Error in SIGINT handling. Exiting...\n");
        exit(1);
    }
}

struct package
{
    int flag;
    char input[64];
};

void main()
{
    signal(SIGINT, sigHandlerWriter);
    int sharedMemoryID;
    struct package* sharedMemoryPointer;
    key_t sharedKey = ftok("./writer.c", 1);

    if ((sharedMemoryID = shmget(sharedKey, sizeof(struct package), S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | IPC_CREAT)) < 0)
    {
        perror("Unable to get shared memory\n");
        exit(1);
    }

    if ((sharedMemoryPointer = shmat(sharedMemoryID, 0, 0)) == (void*)-1)
    {
        perror("Unable to attach\n");
        exit(1);
    }

    sharedMemoryPointer->flag = 0;

    while (1) {

        char temp[64];
        printf("Message [Max 64 Charachters]: ");
        fgets(temp, sizeof(temp), stdin);
        strcpy(sharedMemoryPointer->input, temp);

        sharedMemoryPointer->flag = 1;
        while (sharedMemoryPointer->flag);

        if (SHUTDOWNFLAG) {
            if (shmdt(sharedMemoryPointer) < 0)
            {
                perror("Unable to detach\n");
                exit(1);
            }
            if (shmctl(sharedMemoryID, IPC_RMID, 0) < 0)
            {
                perror("Unable to deallocate\n");
                exit(1);
            }
            exit(0);
        }

    }

}
