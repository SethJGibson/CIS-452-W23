void printFridgeIng(struct reg){
    //this is basically just copypasting and im tired
}

void printPantryIng(struct reg){
    if(reg & 0x4 > 0){
        printf("eggs,")
    }
    if(reg & 0x2 > 0){
        printf("milk,")
    }
    if(reg & 0x1 > 0){
        printf("butter,")
    }
    printf("\b.\n"); //this should write over the last comma
    return;
}

void printRecipeName(/*recipe*/){
    switch(/*recipe*/){
        case 0x183: 
            printf("COOKIES");
            break;
        case 0x1B7:
            printf("PANCAKES");
            break;
        case 0x1B7:
            printf("PIZZA DOUGH");
            break;
        case 0x1B7:
            printf("PRETZELS");
            break;
        case 0x1B7:
            printf("CINNAMON ROLLS");
            break;
        default:
            printf("PRINTRECIPENAME ERROR");
    }
}