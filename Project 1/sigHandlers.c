void sigHandlerParent(int SigNum){
    switch(sigNum){
        case SIGINT:
            //ctrl c
            //close pipes
            exit(0);
        case SIGUSR1:
            //take new input
            INPUTFLAG = false;
    }
    
}

void sigHandlerChild(int sigNum){
    switch(sigNum){
        case SIGINT:
            //ctrl c
            //close pipes
            exit(0);
    }
}
