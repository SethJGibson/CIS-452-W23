#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int shutDownFlag = 0;

struct package {
	int flag;
	char input[64];
};

void sigIntHandler(int sigNum) {
	shutDownFlag = 1;
}

int main()
{
	signal(SIGINT, sigIntHandler);

	int shmId;
	key_t key;
	struct package* sharedMemoryPtr;
	sharedMemoryPtr->flag = 0;

	key = ftok("./writer.c", 2);
	if (key < 0) {
		perror("Unable to generate producer.c key\n");
		exit(1);
	}

	if ((shmId = shmget(key, sizeof(struct package), S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | IPC_CREAT)) < 0) {
		perror("Unable to get shared memory\n");
		exit(1);
	}

	if ((sharedMemoryPtr = shmat(shmId, 0, 0)) == (void*)-1) {
		perror("Unable to attach\n");
		exit(1);
	}

	while (1) {
		while (sharedMemoryPtr->flag == 0);
		printf("Input from Shared Memory: %s\n", sharedMemoryPtr->input);

		memset(sharedMemoryPtr->input, 0, sizeof(sharedMemoryPtr->input));
		sharedMemoryPtr->flag = 0;

		if (shutDownFlag) {
			if (shmdt(sharedMemoryPtr) < 0) {
				perror("Unable to detach\n");
				exit(1);
			}
		}
	}

	return 0;
}
