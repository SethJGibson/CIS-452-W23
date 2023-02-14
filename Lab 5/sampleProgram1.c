#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define FOO 4096

int main()
{
    int shmId;
    char *sharedMemoryPtr;
    struct shmid_ds shmidStruct;

    if ((shmId = shmget(IPC_PRIVATE, FOO, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
    {
        perror("Unable to get shared memory\n");
        exit(1);
    }
    if ((sharedMemoryPtr = shmat(shmId, 0, 0)) == (void *)-1)
    {
        perror("Unable to attach\n");
        exit(1);
    }
    printf("Value a: %p\t Value b: %p\t Segment ID: %d\n", (void *)sharedMemoryPtr, (void *)sharedMemoryPtr + FOO, shmId);
    pause();
    
    if (shmdt(sharedMemoryPtr) < 0)
    {
        perror("Unable to detach\n");
        exit(1);
    }
    if (shmctl(shmId, IPC_RMID|IPC_STAT, &shmidStruct) < 0)
    {
        perror("Unable to deallocate\n");
        exit(1);
    }
    printf("Segment size: %d\n", shmidStruct.shm_segsz);
    //printf("Segment ID: %d\n", shmId);
    //pause();

    return 0;
}