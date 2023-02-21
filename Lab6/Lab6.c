#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#define SIZE 16

int main(int argc, char* argv[])
{
	int status;
	long int i, loop, temp, * sharedMemoryPointer;
	int sharedMemoryID;
	key_t key;
	//int semFlag, nsems, semID;
	int semID;
	struct sembuf sops[2];
	
	pid_t pid;
	loop = atoi(argv[1]);
	sharedMemoryID = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (sharedMemoryID < 0) {
		perror("Unable to obtain shared memory\n");
		exit(1);
	}
	sharedMemoryPointer = shmat(sharedMemoryID, 0, 0);
	if (sharedMemoryPointer == (void*)-1) {
		perror("Unable to attach\n");
		exit(1);
	}
	sharedMemoryPointer[0] = 0;
	sharedMemoryPointer[1] = 1;
	
	// INIT SEMAPHORES
	//if (semID = semget(key, nsems, semFlag) == -1) {
	if ((semID = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("semget failed. Exiting...");
		exit(1);
	}
	semctl(semID, 0, SETVAL, 1);
	
	sops[0].sem_num = 0;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;
	sops[1].sem_num = 0;
	sops[1].sem_op = 1;
	sops[1].sem_flg = 0;
	
	// FORK PROCS
	pid = fork();
	if (pid < 0) {
		printf("Fork failed\n");
	}
	
	/* CRITICAL SECTION START */
	
	if (pid == 0) { // Child
		for (i = 0; i < loop; i++) {
			semop(semID, &sops[0], 1);
			
			int temp = sharedMemoryPointer[0];
			sharedMemoryPointer[0] = sharedMemoryPointer[1];
			sharedMemoryPointer[1] = temp;
			
			semop(semID, &sops[1], 1);
		}
		if (shmdt(sharedMemoryPointer) < 0) {
			perror("Unable to detach\n");
			exit(1);
		}
		exit(0);
	}
	else {	// Parent
		for (i = 0; i < loop; i++) {
			semop(semID, &sops[0], 1);
			
			int temp = sharedMemoryPointer[0];
			sharedMemoryPointer[0] = sharedMemoryPointer[1];
			sharedMemoryPointer[1] = temp;
			
			semop(semID, &sops[1], 1);
		}
	}
	wait(&status);
	printf("Values: %li\t%li\n", sharedMemoryPointer[0], sharedMemoryPointer[1]);
		
	/* CRITICAL SECTION END */
	
	if (semctl(semID, 0, IPC_RMID) < 0) {
		perror("Unable to deallocate semaphore\n");
		exit(1);
	}
	
	if (shmdt(sharedMemoryPointer) < 0) {
		perror("Unable to detach shared mem\n");
		exit(1);
	}
	if (shmctl(sharedMemoryID, IPC_RMID, 0) < 0) {
		perror("Unable to deallocate shared mem\n");
		exit(1);
	}
	return 0;
}
