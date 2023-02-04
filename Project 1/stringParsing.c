#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main() {
    char apple[256] = "12\nhello";
    printf("%s\n", apple);
    char *token;
    char dest[2];
    char msg[256];
    token = strtok(apple, "\n");
    printf("%s\n", token);
    strcpy(dest, token);
    token = strtok(NULL, "\n");
    printf("%s\n", token);
    strcpy(msg, token);
    printf("%d msg: %s\n", atoi(dest), msg);
    printf("%s\n", apple);
}
