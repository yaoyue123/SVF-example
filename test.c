#include <stdio.h>
#include <stdlib.h>
char * main(){
    char *p; 
    p  = malloc(4);
    p  = malloc(4);
    p  = malloc(4);
    return p;
}


void test2(char* p, char*q){
    char *a = "dfsfd";
    
    p = malloc(4);

    q = a;
}