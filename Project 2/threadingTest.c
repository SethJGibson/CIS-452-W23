#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void bakerTest() {
    printf("REACHED bakerTest()!\n");
    exit(0);
}

void threadHandler(int pid) {

    kill(pid, SIGUSR1);
}

int main()
{
    printf("Hello World\n");
    int pid = getpid();

    pthread_t tid;
    pthread_create(&tid, NULL, threadHandler, (void *)pid); // Can we pass two arguments?

    signal(SIGUSR1, bakerTest);
    while (1);

    return 0;
}
