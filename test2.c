char * test2(char* p, char **q){
    char *a = "dfsfd";
    
    p = malloc(4);

    *q = a;

    return p;
}