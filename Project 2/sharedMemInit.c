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

struct reg {
	unsigned int ing : 10;	// 10 bits for the ingredient register 
};

int main(int argc, char* argv[])
{
	int status;
	long int i, loop, temp, * sharedMemoryPointer;
	int sharedMemoryID;
	key_t key;
	//int semFlag, nsems, semID;
	int semID;
	struct sembuf sops[2];

	struct reg onHand, recipe, cookbook[5];
	struct reg pantry, fridge1, fridge2;

	onHand.ing = 0;

	cookbook[0].ing = 0x183;	// COOKIES:			110000011
	cookbook[1].ing = 0x1B7;	// PANCAKES:		110110111
	cookbook[2].ing = 0xD0;		// PIZZA DOUGH:		011010000
	cookbook[3].ing = 0x1F4;	// PRETZELS:		111110100
	cookbook[4].ing = 0x19E;	// CINNAMON ROLLS:	110011110

	pid_t pid;

	int shmSize = sizeof(struct reg) * 3;
	sharedMemoryID = shmget(IPC_PRIVATE, shmSize, IPC_CREAT | S_IRUSR | S_IWUSR);
	if (sharedMemoryID < 0) {
		perror("Unable to obtain shared memory\n");
		exit(1);
	}

	struct reg* storage = (struct reg*)shmat(sharedMemoryID, NULL, 0);
	if (storage == (void*)-1) {
		perror("Unable to attach\n");
		exit(1);
	}

	storage[0].ing = 0x1F8;
	storage[1].ing = 0x7;
	storage[2].ing = 0x7;

	// FORK PROCS
	pid = fork();
	if (pid < 0) {
		printf("Fork failed\n");
	}

	/* CRITICAL SECTION START */

	if (pid == 0) { // Child
		recipe.ing = cookbook[3].ing; // TODO: Random selection
		onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);	// Check Pantry
		onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);	// Check Fridge

		if (onHand.ing == recipe.ing)
			printf("Bit register works!\n");
		else
			printf("Bit register failed! -> Child onHand - %x\n", onHand.ing);
		exit(0);
	}
	else {	// Parent
		recipe.ing = cookbook[1].ing; // TODO: Random selection
		onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);	// Check Pantry
		onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);	// Check Fridge

		if (onHand.ing == recipe.ing)
			printf("Bit register works!\n");
		else
			printf("Bit register failed! -> Parent onHand - %x\n", onHand.ing);
	}
	wait(&status);

	/* CRITICAL SECTION END */

	if (shmdt(storage) < 0) {
		perror("Unable to detach shared mem\n");
		exit(1);
	}
	if (shmctl(sharedMemoryID, IPC_RMID, 0) < 0) {
		perror("Unable to deallocate shared mem\n");
		exit(1);
	}
	return 0;
}
