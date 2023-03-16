#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE 256
int main()
{
    char *data1;
    data1 = malloc(SIZE);
    //char *data2;
    //data2 = malloc(1);
    printf("Please input username: ");
    scanf("%s", data1);
    //data2 = (char *)realloc(data1, strlen(data1));
    data1 = (char *)realloc(data1, strlen(data1));
    printf("You entered: [%s]\n", data1);
    free(data1);
    //free(data2);
    return 0;
}
