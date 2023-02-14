#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

struct myStruct
{
    int myInt;
    char myString[40];
};

void
main()
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

    sharedMemoryPointer->myInt = 42;
    char *temp = "Not Hello World";
    strcpy(sharedMemoryPointer->myString, temp);

    if (shmdt(sharedMemoryPointer) < 0)
    {
        perror("Unable to detach\n");
        exit(1);
    }
}