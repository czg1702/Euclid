#include <stdio.h>
#include <stdlib.h>

void max_mem();

int main(int argc, char *argv[])
{
    max_mem();

    return 0;
}

void max_mem()
{
    size_t sz = 1024 * 1024; // 1M
    char *addr;
    while (1)
    {
        addr = malloc(sz);
        if (addr)
        {
            printf("%lu\n", sz);
            free(addr);
        }
        else
        {
            sz -= 1024 * 1024;
            printf("---\n");
            break;
        }

        sz += 1024 * 1024;
    }

    addr = malloc(sz);
    if (!addr)
        exit(1);
    free(addr);

    sz = sz / 10;
    int i;
    for (i = 0; i < 30; i++)
    {
        addr = malloc(sz);
        printf("% 8d >>> %p % 32lu\n", i, addr,   (size_t) addr);
    }

    printf("test completed.\n");
}