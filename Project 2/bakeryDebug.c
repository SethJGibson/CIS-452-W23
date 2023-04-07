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
#include <string.h>

// https://stackoverflow.com/questions/523724/c-c-check-if-one-bit-is-set-in-i-e-int-variable
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

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

void* bakingTime(void* num); //Prototype so the function can go below main
void printFridgeIng(struct reg workingReg);
void printPantryIng(struct reg workingReg);
char* printRecipeName(struct reg recipe);

int mixers, pantry, fridges, bowls, spoons, oven;
struct sembuf getSem = { 0, -1, 0 };
struct sembuf returnSem = { 0, 1, 0 };

int main() {

    /////////////////////////////
    //SEMAPHORES
    /////////////////////////////

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

    printf("[BAKER #%d] Starting...\n", bakerNum);

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
        char recipeStr[25];
        strcpy(recipeStr, printRecipeName(recipe));

        printf("[BAKER #%d] chooses %s!\n", bakerNum, recipeStr);

        if ((recipe.ing & storage[1].ing) > 0) {                    // If recipe needs ingredients from pantry,
            //printf("[BAKER #%d:%s] Going to Fridge...\n", bakerNum, recipeStr);
            semop(fridges, &getSem, 1);

            printf("[BAKER #%d:%s]Fridge\n", bakerNum, recipeStr);
            //printf("[BAKER #%d:%s] Grabbing ingredients: ", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);
            printFridgeIng(onHand);

            //printf("[BAKER #%d:%s] Left Fridge\n", bakerNum, recipeStr);
            semop(fridges, &returnSem, 1);
        }

        if ((recipe.ing & storage[0].ing) > 0) {                    // If recipe needs ingredients from either fridge,
            //printf("[BAKER #%d:%s] Going to Pantry...\n", bakerNum, recipeStr);
            semop(pantry, &getSem, 1);

            printf("[BAKER #%d:%s]Pantry:\n", bakerNum, recipeStr);
            //printf("[BAKER #%d:%s]", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);
            printPantryIng(onHand);

            //printf("[BAKER #%d:%s] Left Pantry\n", bakerNum, recipeStr);
            semop(pantry, &returnSem, 1);
        }

        semop(mixers, &getSem, 1);
        //printf("[BAKER #%d:%s] Got Mixer.\n", bakerNum, recipeStr);
        semop(bowls, &getSem, 1);
        //printf("[BAKER #%d:%s] Got Bowl.\n", bakerNum, recipeStr);
        semop(spoons, &getSem, 1);
        //printf("[BAKER #%d:%s] Got Spoon.\n", bakerNum, recipeStr);

        //printf("[BAKER #%d:%s] Mixing...\n", bakerNum, recipeStr);
        sleep(1);

        semop(mixers, &returnSem, 1);
        semop(bowls, &returnSem, 1);
        semop(spoons, &returnSem, 1);

        semop(oven, &getSem, 1);
        //printf("[BAKER #%d:%s] Got Oven.\n", bakerNum, recipeStr);
        //printf("[BAKER #%d:%s] Baking...\n", bakerNum, recipeStr);
        sleep(1);
        semop(oven, &returnSem, 1);

        onHand.ing = 0;
    }

    // BAKER LOGIC END

    printf("[BAKER #%d] Finished baking!\n", bakerNum);

    return NULL;
}

void printPantryIng(struct reg workingReg) {
    printf("reg: %d\n", workingReg.ing);
    //CHECK_BIT(temp, n)
    printf("%d", CHECK_BIT(workingReg.ing, 8) > 0);
    if (CHECK_BIT(workingReg.ing, 8)) {
        //printf("Flour, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 7) > 0);
    if (CHECK_BIT(workingReg.ing, 7)) {
        //printf("Sugar, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 6) > 0);
    if (CHECK_BIT(workingReg.ing, 6)) {
        //printf("Yeast, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 5) > 0);
    if (CHECK_BIT(workingReg.ing, 5)) {
        //printf("Baking soda, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 4) > 0);
    if (CHECK_BIT(workingReg.ing, 4)) {
        //printf("Salt, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 3) > 0);
    if (CHECK_BIT(workingReg.ing, 3)) {
        //printf("Cinnamon, ");
    }
    printf("\n"); //this should write over the last comma
    //printf("%x\n", workingReg.ing);
    return;
}

void printFridgeIng(struct reg workingReg) {
    printf("reg: %d\n", workingReg.ing);
    printf("%d", CHECK_BIT(workingReg.ing, 2) > 0);
    if (CHECK_BIT(workingReg.ing, 2)) {
        //printf("Eggs, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 1) > 0);
    if (CHECK_BIT(workingReg.ing, 1)) {
        //printf("Milk, ");
    }
    printf("%d", CHECK_BIT(workingReg.ing, 0) > 0);
    if (CHECK_BIT(workingReg.ing, 0)) {
        //printf("Butter, ");
    }
    printf("\n"); //this should write over the last comma
    //printf("%x\n", workingReg.ing);
    return;
}

char* printRecipeName(struct reg recipe) {
    switch (recipe.ing) {
    case 0x183:
        return "COOKIES";
    case 0x1B7:
        return "PANCAKES";
    case 0xD0:
        return "PIZZA DOUGH";
    case 0x1F4:
        return "PRETZELS";
    case 0x19E:
        return "CINNAMON ROLLS";
    default:
        return "PRINTRECIPENAME ERROR";
    }
}
