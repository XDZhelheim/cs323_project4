#include <stdio.h>
#include <string.h>

int main()
{
    char *a=NULL;
    int b;
    getline(&a, &b, stdin);
    printf("%s\n", a);
    printf("%d\n", strlen(a));
}