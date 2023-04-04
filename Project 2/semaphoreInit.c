struct sembuf getSem;
getSem.sem_num = 0; //Which semaphore in a set to perform the op on.
getSem.sem_op = -1; //Value to modify the semVal by.
getSem.sem_flg = 0; //IPC_NOWAIT and/or SEM_UNDO

struct sembuf returnSem;
returnSem.sem_num = 0;
returnSem.sem_op = 1;
returnSem.sem_flg = 0;

//We can probably just use semtimedop() instead of this
//struct sembuf getSemNoWait;
//returnSem.sem_num = 0;
//returnSem.sem_op = -1;
//returnSem.sem_flg = IPC_NOWAIT;

//Initialize semaphores
if ((mixers = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget mixers. Exiting...\n");
		exit(1);
}
if ((pantry = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget pantry. Exiting...\n");
		exit(1);
}
if ((fridges = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget fridges. Exiting...\n");
		exit(1);
}
if ((bowls = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget bowls. Exiting...\n");
		exit(1);
}
if ((spoons = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget spoons. Exiting...\n");
		exit(1);
}
if ((oven = semget(IPC_PRIVATE, 1, 0600)) == -1) {
		perror("Failed to semget oven. Exiting...\n");
		exit(1);
}

//Initialize semVals - We could/should add errorchecking to these
semctl(mixers, 0, SET_VAL, 2);
semctl(pantry, 0, SET_VAL, 1);
semctl(fridges, 0, SET_VAL, 2);
semctl(bowls, 0, SET_VAL, 3);
semctl(spoons, 0, SET_VAL, 5);
semctl(oven, 0, SET_VAL, 1);

