//Parent Process loop
signal(SIGINT, sigHandlerChild);
while(1){
    
    INPUTFLAG = true;
    char message[256];
    char dest[16];
    
    printf("Message [Max 256 Charachters]: ");
    fgets(message, sizeof(message), stdin);
    
    printf("\nDestination [%d-%d]: ", 1, numChildren);
    fgets(dest, sizeof(dest), stdin);
    
    if(atoi(dest) < 1 || atoi(dest) > numChildren){
        printf("\nInvalid Destination.\n");
        continue;
    }
    
    strcat(dest, ":");
    strcat(dest, message);
    write(p[1], dest, MSGSIZE);

    while(INPUTFLAG);
}
