//Child process loop
signal(SIGINT, sigHandlerParent);
char apple[256];
while(1){
    //loop until pipe contains a message
    while(read(p[0], apple, MSGSIZE);
    
    //deconstruct apple
    char *token;
    char dest[16];
    char msg[256];
    token = strtok(apple, ":");
    strcpy(dest, token);
    token = strtok(NULL, ":");
    strcpy(msg, token);
    
    //check if message is for this node
    if(atoi(dest) == nodeNum){
        printf("Node %d got message: %s\n", nodeNum, msg);
        kill(parentPID, SIGUSR1);
        write(p[1], NULL, MSGSIZE);
        continue;
    }else{
        printf("Node %d passes message.\n", nodeNum);
        write(p[1], apple, MSGSIZE);
        continue;
    }
}
