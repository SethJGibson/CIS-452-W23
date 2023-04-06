if(recipe % 8 > 0){
    printf("BAKER #%d: Going to Fridge...\n", bakerNum);
    semop(fridge, getSem, 1);
    printf("BAKER #%d: Entered Fridge\n", bakerNum);
    baker = baker | fridge;
    printf("BAKER #%d: Left Fridge\n", bakerNum);
    semop(fridge, returnSem, 1);
}
if(recipe > 7){
    printf("BAKER #%d: Going to Pantry...\n", bakerNum);
    semop(pantry, getSem, 1);
    printf("BAKER #%d: Entered Pantry\n", bakerNum);
    baker = baker | pantry;
    printf("BAKER #%d: Left Pantry\n", bakerNum);
    semop(pantry, returnSem, 1);
}
//TODO need to print WHAT ingredients are being obtained

semop(mixer, getSem, 1);
printf("BAKER #%d: Got Mixer.\n", bakerNum);
semop(bowl, getSem, 1);
printf("BAKER #%d: Got Bowl.\n", bakerNum);
semop(spoon, getSem, 1);
printf("BAKER #%d: Got Spoon.\n", bakerNum);
sleep(1);

printf("BAKER #%d: Mixing %s...\n", bakerNum, currentRecipe)
sleep(1);

semop(mixer, returnSem, 1);
semop(bowl, returnSem, 1);
semop(spoon, returnSem, 1);

semop(oven, getSem, 1);
printf("BAKER #%d: Got Oven.\n", bakerNum);
sleep(1);
printf("BAKER #%d: Baking %s...\n", bakerNum, currentRecipe);
sleep(1);
semop(oven, returnSem, 1);

