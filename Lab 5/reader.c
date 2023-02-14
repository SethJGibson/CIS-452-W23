#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

struct package
{
    int flag;
    char input[64];
};

void main()
{

    int sharedMemoryID;
    struct myStruct *sharedMemoryPointer;

    key_t sharedKey = ftok("/producer.c", 1);

    if ((sharedMemoryID = shmget(sharedKey, sizeof(struct myStruct), S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | IPC_CREAT)) < 0)
    {
        perror("Unable to get shared memory\n");
        exit(1);
    }

    if ((sharedMemoryPointer = shmat(sharedMemoryID, 0, 0)) == (void *)-1)
    {
        perror("Unable to attach\n");
        exit(1);
    }

    printf("Int: %d\n", sharedMemoryPointer->myInt);
    printf("String: %s\n", sharedMemoryPointer->myString);

    if(shmdt(sharedMemoryPointer) < 0)
    {
        perror("Unable to detach\n");
        exit(1);
    }

    if (shmctl(sharedMemoryID, IPC_RMID, 0) < 0)
    {
        perror("Unable to deallocate\n");
        exit(1);
    }
}