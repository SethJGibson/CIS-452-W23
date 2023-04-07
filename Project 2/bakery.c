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

// 10 bits for the ingredient register 
struct reg {
    unsigned int ing : 10;	
};

//Struct to hold recipe registers
struct reg cookbook[5] = {
    {.ing = 0x183},
    {.ing = 0x1B7},
    {.ing = 0xD0},
    {.ing = 0x1F4},
    {.ing = 0x19E}
};

//Struct to hold fridge/pantry data (what ingredients they contain)
struct reg storage[2] = {
    {.ing = 0x1F8}, //Pantry
    {.ing = 0x7} //Fridge
};

//Function prototypes
void* bakingTime(void* num); //Prototype so the function can go below main
void printFridgeIng(struct reg workingReg);
void printPantryIng(struct reg workingReg);
char* printRecipeName(struct reg recipe);

//Semaphore names & operations
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
    char numInput[4];   // User input buffer

    do {
        printf("Number of bakers [1-255]: "); //technically up to 999
        fgets(numInput, sizeof(numBakers) + 1, stdin); //will cause problems if >999
        numBakers = atoi(numInput);
    } while (numBakers < 1); //Must be at least 1 baker

    pthread_t threads[numBakers + 1];
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
    free(num); //frees bakerNum variable malloc'd in main

    printf("[BAKER #%d] Starting...\n", bakerNum);

    // BAKER LOGIC START

    struct reg onHand, recipe; //Register for what baker has & what recipe uses
    onHand.ing = 0;
    recipe.ing = 0;

    //Randomize recipe completion order
    int order[] = { 0, 1, 2, 3, 4 };
    for (int i = 0; i < 5; i++) {
        int temp = order[i];
        int randSelect = rand() % 5;
        order[i] = order[randSelect];
        order[randSelect] = temp;
    }

    //Baking loop
    for (int i = 0; i < 5; i++) {
        //Set recipe, get string of recipe name
        recipe = cookbook[order[i]];
        char recipeStr[25];
        strcpy(recipeStr, printRecipeName(recipe));

        printf("[BAKER #%d] chooses %s!\n", bakerNum, recipeStr);

        // If recipe needs ingredients from one of the fridges:
        if ((recipe.ing & storage[1].ing) > 0) {    
            printf("[BAKER #%d:%s] Going to Fridge...\n", bakerNum, recipeStr);
            semop(fridges, &getSem, 1);

            printf("[BAKER #%d:%s] Entered Fridge\n", bakerNum, recipeStr);
            printf("[BAKER #%d:%s] Grabbing ingredients: ", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);
            printFridgeIng(onHand);

            printf("[BAKER #%d:%s] Left Fridge\n", bakerNum, recipeStr);
            semop(fridges, &returnSem, 1);
        }

        // If recipe needs ingredients from the pantry:
        if ((recipe.ing & storage[0].ing) > 0) {  
            printf("[BAKER #%d:%s] Going to Pantry...\n", bakerNum, recipeStr);
            semop(pantry, &getSem, 1);

            printf("[BAKER #%d:%s] Entered Pantry\n", bakerNum, recipeStr);
            printf("[BAKER #%d:%s] Grabbing ingredients: ", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);
            printPantryIng(onHand);

            printf("[BAKER #%d:%s] Left Pantry\n", bakerNum, recipeStr);
            semop(pantry, &returnSem, 1);
        }

        semop(mixers, &getSem, 1);
        printf("[BAKER #%d:%s] Got Mixer.\n", bakerNum, recipeStr);
        semop(bowls, &getSem, 1);
        printf("[BAKER #%d:%s] Got Bowl.\n", bakerNum, recipeStr);
        semop(spoons, &getSem, 1);
        printf("[BAKER #%d:%s] Got Spoon.\n", bakerNum, recipeStr);

        printf("[BAKER #%d:%s] Mixing...\n", bakerNum, recipeStr);
        sleep(1);

        semop(mixers, &returnSem, 1);
        semop(bowls, &returnSem, 1);
        semop(spoons, &returnSem, 1);

        semop(oven, &getSem, 1);
        printf("[BAKER #%d:%s] Got Oven.\n", bakerNum, recipeStr);
        printf("[BAKER #%d:%s] Baking...\n", bakerNum, recipeStr);
        sleep(1);
        semop(oven, &returnSem, 1);

        onHand.ing = 0;
    }

    // BAKER LOGIC END

    printf("[BAKER #%d] Finished baking!\n", bakerNum);

    return NULL;
}

/////////////////////////
// HELPER FUNCTIONS
/////////////////////////

//Prints what pantry ingredients a register contains
void printPantryIng(struct reg workingReg) {
    //CHECK_BIT(temp, n)
    if (CHECK_BIT(workingReg.ing, 8)) {printf("Flour, ");}
    if (CHECK_BIT(workingReg.ing, 7)) {printf("Sugar, ");}
    if (CHECK_BIT(workingReg.ing, 6)) {printf("Yeast, ");}
    if (CHECK_BIT(workingReg.ing, 5)) {printf("Baking soda, ");}
    if (CHECK_BIT(workingReg.ing, 4)) {printf("Salt, ");}
    if (CHECK_BIT(workingReg.ing, 3)) {printf("Cinnamon, ");}
    printf("\b\b.\n"); //this should write over the last comma
    return;
}

//Prints what fridge ingredients a register contains
void printFridgeIng(struct reg workingReg) {
    if (CHECK_BIT(workingReg.ing, 2)) {printf("Eggs, ");}
    if (CHECK_BIT(workingReg.ing, 1)) {printf("Milk, ");}
    if (CHECK_BIT(workingReg.ing, 0)) {printf("Butter, ");}
    printf("\b\b.\n"); //this should write over the last comma
    return;
}

//Given a register, returns what recipe the register is for
char* printRecipeName(struct reg recipe) {
    switch (recipe.ing) {
    case 0x183: return "COOKIES";
    case 0x1B7: return "PANCAKES";
    case 0xD0: return "PIZZA DOUGH";
    case 0x1F4: return "PRETZELS";
    case 0x19E: return "CINNAMON ROLLS";
    default: return "PRINTRECIPENAME ERROR";
    }
}
