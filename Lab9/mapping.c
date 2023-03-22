#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//from
// https://stackoverflow.com/questions/7370407/get-the-start-and-end-address-of-text-section-in-an-executable
extern char __executable_start;

int global1 = 9348729;
char* global2 = "hello im a global var";
bool global3 = true;

int func1(){
    return 23948723;
}

char* func2(){
    return "hello im a function";
}

bool func3(){
    return false;
}

int main(){

    int data1 = 1;
    int data2 = 2;
    int data3 = 1234567890;

    printf("Func1 address: %p\n", &func1);
    printf("Func2 address: %p\n", &func2);
    printf("Func3 address: %p\n", &func3);
    printf("Global1 address: %p\n", &global1);
    printf("Global2 address: %p\n", &global2);
    printf("Global3 address: %p\n", &global3);
    printf("Data1 address: %p\n", &data1);
    printf("Data2 address: %p\n", &data2);
    printf("Data3 address: %p\n", &data3);
    printf("Text _start address: 0x%lx\n", (unsigned long)&__executable_start);
    
}
