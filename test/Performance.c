#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    long i;
    double val;

    for (i = 0; i < 1000000000; i++)
    {
        val = (i + 1) * 1.0000001;
        // if (i % 10000000 == 0)
        //     printf("%ld\t%f\n", i, val);
    }

    return 0;
}