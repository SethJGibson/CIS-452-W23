#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int numReqs;

//Struct is needed so threads don't overwrite data
struct filePathWrapper {
    char filePath[64];
};

//Signal Handler: Prints # of request and exits
void sigHandler(int sigNum){
    switch (sigNum) {
	case SIGINT:
		printf("\nProcess Exiting.\nFile Requests Recieved: %d\n", numReqs);
        exit(0);
		break;
	}
}

//Worker thread program, gets filePath from the struct, sleeps, then prints and exits.
void* retrieveFile(void *var){

    struct filePathWrapper *wpr = (struct filePathWrapper*)var;
    int randInt = rand() % 10 + 1; //1-10

    sleep((randInt > 3 ? 1 : (7 + rand() % 4)));

    printf("\nRetrieved file at filepath %s\n", wpr->filePath);
    free(wpr);
    return NULL;

}

int main(){

    signal(SIGINT, sigHandler);

    //Loops until interrupt (ctrl-c)
    while(1){

        struct filePathWrapper *wpr = malloc(sizeof *wpr);
        char temp[64];

        //Read string from stdin, if nothing given loop again
        printf("\nEnter a file to retrieve: ");
        fgets(temp, sizeof(temp), stdin);
        strcpy(wpr->filePath, temp); //structs can't take in strings directly apparently?
        if(wpr->filePath[0] == '\n'){
            continue;
        }

        //Thread created & run with fileRetrieve and struct passed.
        pthread_t tid;
        int threadStatus = pthread_create (&tid, NULL, retrieveFile, (void *)wpr);
        if (threadStatus != 0){
            fprintf (stderr, "Thread create error %d: %s\n", threadStatus, strerror(threadStatus));
            exit (1);
        }

        //Increment requests recieved (not returned!)
        numReqs = numReqs + 1;
    }
}

