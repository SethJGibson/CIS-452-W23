/////////////////////////////
//David Geisel & Seth Gibson
//OS Project 2
/////////////////////////////

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>

void* bakingTime(); //Prototype so the function can go below main

struct reg {
    unsigned int ing : 10;	// 10 bits for the ingredient register 
};

struct reg cookbook[5] = {
    {.ing = 0x183},
    {.ing = 0x1B7},
    {.ing = 0xD0},
    {.ing = 0x1F4},
    {.ing = 0x19E}
};

struct reg storage[2] = {
    {.ing = 0x1F8},
    {.ing = 0x7}
};

//struct ThreadArgs {
//    struct sembuf* getSem;
//    struct sembuf* returnSem;
//
//    int* mixers, pantry, fridges, bowls, spoons, oven;
//    int* bakerNum;
//};

int mixers, pantry, fridges, bowls, spoons, oven;
struct sembuf getSem = { 0, -1, 0 };
struct sembuf returnSem = { 0, 1, 0 };

int main() {

    /////////////////////////////
    //SEMAPHORES
    /////////////////////////////

    //Semaphore operations
    //struct sembuf getSem;
    //getSem.sem_num = 0; //Which semaphore in a set to perform the op on.
    //getSem.sem_op = -1; //Value to modify the semVal by.
    //getSem.sem_flg = 0; //IPC_NOWAIT and/or SEM_UNDO

    //struct sembuf returnSem;
    //returnSem.sem_num = 0;
    //returnSem.sem_op = 1;
    //returnSem.sem_flg = 0;

    //Initialize semaphores
    //int mixers, pantry, fridges, bowls, spoons, oven;
    if ((mixers = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget mixers.\n");
        exit(1);
    }
    if ((pantry = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget pantry.\n");
        exit(1);
    }
    if ((fridges = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget fridges.\n\n");
        exit(1);
    }
    if ((bowls = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget bowls.\n\n");
        exit(1);
    }
    if ((spoons = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget spoons.\n\n");
        exit(1);
    }
    if ((oven = semget(IPC_PRIVATE, 1, 0600)) == -1) {
        perror("ERROR: semget oven.\n\n");
        exit(1);
    }

    //Initialize semVals - We could add errorchecking to these
    semctl(mixers, 0, SETVAL, 2);
    semctl(pantry, 0, SETVAL, 1);
    semctl(fridges, 0, SETVAL, 2);
    semctl(bowls, 0, SETVAL, 3);
    semctl(spoons, 0, SETVAL, 5);
    semctl(oven, 0, SETVAL, 1);

    /////////////////////////////
    //SHARED MEMORY
    /////////////////////////////

        //TODO Put shared mem stuff here

    /////////////////////////////
    //THREAD INIT
    /////////////////////////////

    int numBakers;      // Will hold the number of bakers/threads taken from input
    char numInput[4];   // Holds user input - in project1 this was only 1 long? honestly not sure how that even worked

    do {
        printf("Number of bakers [1-255]: "); //technically up to 999
        fgets(numInput, sizeof(numBakers) + 1, stdin); //will cause problems if >999, probably not important to fix tho
        numBakers = atoi(numInput);
    } while (numBakers < 1);

    pthread_t threads[numBakers + 1]; //TODO test if this works without the +1
    for (int i = 0; i <= numBakers - 1; i++) {
        int* bakerNumPtr = malloc(sizeof(int)); //Ensures the thread reads the correct value
        *bakerNumPtr = i + 1;
        if (pthread_create(&threads[i], NULL, &bakingTime, bakerNumPtr) != 0) {
            printf("ERROR: Baker #%d failed to create.\n", i + 1);
            exit(1);
        }
        printf("Baker #%d has been created.\n", i + 1);
    }

    printf("All bakers created.\n");

    for (int i = 0; i <= numBakers - 1; i++) {
        if (pthread_join(threads[i], NULL) != 0) {

        }
        //printf("Parent: thread #%d is done.\n", i + 1);
    }
    printf("All bakers are done.\n");

    return 0;
}

/////////////////////////
//BAKER FUNCTION
/////////////////////////

void* bakingTime(void* num) {

    int bakerNum = *(int*)num;
    free(num);

    printf("BAKER #%d: Starting...\n", bakerNum);

    //sleep(1); //This is where all the actual stuff goes
    // BAKER LOGIC START

    struct reg onHand, recipe;
    onHand.ing = 0;
    recipe.ing = 0;

    int order[] = { 0, 1, 2, 3, 4 };
    for (int i = 0; i < 5; i++) {
        int temp = order[i];
        int randSelect = rand() % 5;
        order[i] = order[randSelect];
        order[randSelect] = temp;
    }

    for (int i = 0; i < 5; i++) {
        recipe = cookbook[order[i]];

        // THIS IS WHERE THE LOGIC CHUNK WILL GO
        if ((recipe.ing & storage[0].ing) > 0) {                    // If recipe needs ingredients from pantry,
            printf("BAKER #%d: Going to Fridge...\n", bakerNum);
            semop(fridges, &getSem, 1);                              // PROBLEM! Semaphores arent here in the thread function!

            printf("BAKER #%d: Entered Fridge\n", bakerNum);
            /*baker = baker | fridges;*/
            onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);

            printf("BAKER #%d: Left Fridge\n", bakerNum);
            semop(fridges, &returnSem, 1);
        }

        if ((recipe.ing & storage[1].ing) > 0) {                    // If recipe needs ingredients from either fridge,
            printf("BAKER #%d: Going to Pantry...\n", bakerNum);
            semop(pantry, &getSem, 1);

            printf("BAKER #%d: Entered Pantry\n", bakerNum);
            //baker = baker | pantry;
            onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);

            printf("BAKER #%d: Left Pantry\n", bakerNum);
            semop(pantry, &returnSem, 1);
        }

        semop(mixers, &getSem, 1);
        printf("BAKER #%d: Got Mixer.\n", bakerNum);
        semop(bowls, &getSem, 1);
        printf("BAKER #%d: Got Bowl.\n", bakerNum);
        semop(spoons, &getSem, 1);
        printf("BAKER #%d: Got Spoon.\n", bakerNum);
        //sleep(1);

        printf("BAKER #%d: Mixing...\n", bakerNum);
        sleep(1);

        semop(mixers, &returnSem, 1);
        semop(bowls, &returnSem, 1);
        semop(spoons, &returnSem, 1);

        semop(oven, &getSem, 1);
        printf("BAKER #%d: Got Oven.\n", bakerNum);
        //printf("BAKER #%d: Baking %s...\n", bakerNum, recipe);
        printf("BAKER #%d: Baking...\n", bakerNum);
        sleep(1);
        semop(oven, &returnSem, 1);
    }

    // BAKER LOGIC END

    printf("BAKER #%d: Finished baking!\n", bakerNum);

    return NULL;
}
