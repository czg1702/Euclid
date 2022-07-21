#include <stdio.h>
#include <stdlib.h>

typedef struct _cube_
{
    char flag;
    char no;
    char no2;
    char no3;
    char no4;
    char name[4];
    unsigned long id;
} Cube;

struct STUDENT
{
    char a;
    int b;
    char c;
} data;

struct STUDENT2
{
    char a;
    int b;
} data2;

int main(int argc, char *argv[])
{
    printf("sizeof(Cube) = %d\n", sizeof(Cube));
    printf("%p, %p, %p\n", &data.a, &data.b, &data.c);
    printf("%d\n", sizeof(data));

    printf("%p, %p\n", &data2.a, &data2.b);
    printf("%d\n", sizeof(data2));
    return 0;
}