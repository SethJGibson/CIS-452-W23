/*---------------------------------------------------------------------------------------------------------------------------
 Author:      David Geisel & Seth J. Gibson
 Course:      CIS 457-02
 Program:     Project 2
 Description: This program utilizes shared memory, multi-threading and semaphores to create a mock "kitchen," bustling 
                    with "baker" processes that have to manage shared use of utensils, appliances and food storage. Each
                    resource is locked behind a semaphore and every baker is prepared to wait until it regains access to
                    each resource before proceeding with its recipe. The program ends when all bakers finish all recipes.
 Sources:	  Set Bit Checking - https://stackoverflow.com/questions/523724/c-c-check-if-one-bit-is-set-in-i-e-int-variable
              Too Many Cooks   - https://www.youtube.com/watch?v=QrGrOK8oZG8&ab_channel=AdultSwim
---------------------------------------------------------------------------------------------------------------------------*/

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

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))     // Quick function for checking which bit in a register is high

struct reg {                                        // 9-bit Register Structs for ingredients
    unsigned int ing : 9;
};

struct reg cookbook[5] = {                          // Cookbook Struct to hold all recipe registers
    {.ing = 0x183},                                 // COOKIES:	        110000011
    {.ing = 0x1B7},                                 // PANCAKES:	    110110111
    {.ing = 0xD0},                                  // PIZZA DOUGH:	    011010000
    {.ing = 0x1F4},                                 // PRETZELS:		111110100
    {.ing = 0x19E}                                  // CINNAMON ROLLS:	110011110
};


struct reg storage[2] = {                           // Food Storage Struct to hold fridge/pantry masks
    {.ing = 0x1F8},                                 // Pantry: 111111000
    {.ing = 0x7}                                    // Fridge: 000000111
};

void* bakingTime(void* num);                        // Thread Function
void printFridgeIng(struct reg workingReg);         // Printing Retrieved Fridge Ingredients 
void printPantryIng(struct reg workingReg);         // Printing Retrieved Pantry Ingredients 
char* printRecipeName(struct reg recipe);           // Printing Selected Recipe Name

int mixers, pantry, fridges, bowls, spoons, oven;   // Semaphore names & operations
struct sembuf getSem = { 0, -1, 0 };
struct sembuf returnSem = { 0, 1, 0 };

int main() {

    printf("TooManyCooks v1.0.0 Online!\n");

    /* Semaphore Variables Initialization */

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

    semctl(mixers, 0, SETVAL, 2);
    semctl(pantry, 0, SETVAL, 1);
    semctl(fridges, 0, SETVAL, 2);
    semctl(bowls, 0, SETVAL, 3);
    semctl(spoons, 0, SETVAL, 5);
    semctl(oven, 0, SETVAL, 1);

    /* Threading Initialization */

    int numBakers;                                          // User Input for Threads Spawned
    char numInput[5];                                       // User Input Buffer

    do {
        printf("Number of bakers [1-255]: ");               
        fgets(numInput, sizeof(numBakers) + 1, stdin); 
        numBakers = atoi(numInput);
    } while (numBakers < 1);                                // Must be at least 1 baker

    pthread_t threads[numBakers + 1];
    for (int i = 0; i <= numBakers - 1; i++) {
        int* bakerNumPtr = malloc(sizeof(int));             // Ensures the thread reads the correct value
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
            // Intentionally Left Blank
        }
    }
    printf("All bakers are done.\n");

    return 0;
}

/////////////////////////
//   BAKER FUNCTION
/////////////////////////

void* bakingTime(void* num) {

    int bakerNum = *(int*)num;
    free(num);                                      // Frees bakerNum variable malloc'd in main

    printf("[BAKER #%d] Starting...\n", bakerNum);

    struct reg onHand, recipe;                      
    onHand.ing = 0;                                 // Register for what baker retrieved
    recipe.ing = 0;                                 // Register for what recipe calls for

    int order[] = { 0, 1, 2, 3, 4 };                
    for (int i = 0; i < 5; i++) {
        int temp = order[i];
        int randSelect = rand() % 5;
        order[i] = order[randSelect];
        order[randSelect] = temp;
    }

    for (int i = 0; i < 5; i++) {                       // Primary Loop to iterate through all recipes

        recipe = cookbook[order[i]];                    // Set recipe, get string of recipe name
        char recipeStr[25];
        strcpy(recipeStr, printRecipeName(recipe));

        printf("[BAKER #%d] chooses %s!\n", bakerNum, recipeStr);

        if ((recipe.ing & storage[1].ing) > 0) {                                        // If recipe needs ingredients from one of the fridges...
            printf("[BAKER #%d:%s] Going to Fridge...\n", bakerNum, recipeStr);
            semop(fridges, &getSem, 1);

            printf("[BAKER #%d:%s] Entered Fridge\n", bakerNum, recipeStr);
            printf("[BAKER #%d:%s] Grabbing ingredients: ", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[1].ing);
            printFridgeIng(onHand);

            printf("[BAKER #%d:%s] Left Fridge. Releasing...\n", bakerNum, recipeStr);
            semop(fridges, &returnSem, 1);
        }

        if ((recipe.ing & storage[0].ing) > 0) {                                        // If recipe needs ingredients from the pantry...
            printf("[BAKER #%d:%s] Going to Pantry...\n", bakerNum, recipeStr);
            semop(pantry, &getSem, 1);

            printf("[BAKER #%d:%s] Entered Pantry\n", bakerNum, recipeStr);
            printf("[BAKER #%d:%s] Grabbing ingredients: ", bakerNum, recipeStr);
            onHand.ing = onHand.ing | (recipe.ing & storage[0].ing);
            printPantryIng(onHand);

            printf("[BAKER #%d:%s] Left Pantry. Releasing...\n", bakerNum, recipeStr);
            semop(pantry, &returnSem, 1);
        }

        semop(mixers, &getSem, 1);                                                      // Mixing with utensils
        printf("[BAKER #%d:%s] Got Mixer.\n", bakerNum, recipeStr);
        semop(bowls, &getSem, 1);
        printf("[BAKER #%d:%s] Got Bowl.\n", bakerNum, recipeStr);
        semop(spoons, &getSem, 1);
        printf("[BAKER #%d:%s] Got Spoon.\n", bakerNum, recipeStr);

        printf("[BAKER #%d:%s] Mixing...\n", bakerNum, recipeStr);
        sleep(1);
        printf("[BAKER #%d:%s] Finished mixing. Releasing mixer, bowl, and spoon...\n", bakerNum, recipeStr);

        semop(mixers, &returnSem, 1);
        semop(bowls, &returnSem, 1);
        semop(spoons, &returnSem, 1);

        semop(oven, &getSem, 1);                                                        // Baking with oven,
        printf("[BAKER #%d:%s] Got Oven.\n", bakerNum, recipeStr);
        printf("[BAKER #%d:%s] Baking...\n", bakerNum, recipeStr);
        sleep(1);
        printf("[BAKER #%d:%s] Finished with oven. Releasing...\n", bakerNum, recipeStr);
        semop(oven, &returnSem, 1);

        printf("[BAKER #%d:%s] Recipe Finished!\n", bakerNum, recipeStr);

        onHand.ing = 0;
    }

    printf("[BAKER #%d] Finished baking!\n", bakerNum);

    return NULL;
}

/////////////////////////
//  HELPER FUNCTIONS
/////////////////////////

void printPantryIng(struct reg workingReg) {                            // Prints what pantry ingredients a given register contains
    if (CHECK_BIT(workingReg.ing, 8)) { printf("Flour, "); }
    if (CHECK_BIT(workingReg.ing, 7)) { printf("Sugar, "); }
    if (CHECK_BIT(workingReg.ing, 6)) { printf("Yeast, "); }
    if (CHECK_BIT(workingReg.ing, 5)) { printf("Baking soda, "); }
    if (CHECK_BIT(workingReg.ing, 4)) { printf("Salt, "); }
    if (CHECK_BIT(workingReg.ing, 3)) { printf("Cinnamon, "); }
    printf("\b\b.\n");
    return;
}

void printFridgeIng(struct reg workingReg) {                            // Prints what fridge ingredients a given register contains
    if (CHECK_BIT(workingReg.ing, 2)) { printf("Eggs, "); }
    if (CHECK_BIT(workingReg.ing, 1)) { printf("Milk, "); }
    if (CHECK_BIT(workingReg.ing, 0)) { printf("Butter, "); }
    printf("\b\b.\n");
    return;
}

char* printRecipeName(struct reg recipe) {                              // Given a register, returns what recipe the register is for
    switch (recipe.ing) {
    case 0x183: return "COOKIES";
    case 0x1B7: return "PANCAKES";
    case 0xD0: return "PIZZA DOUGH";
    case 0x1F4: return "PRETZELS";
    case 0x19E: return "CINNAMON ROLLS";
    default: return "PRINTRECIPENAME ERROR";
    }
}
