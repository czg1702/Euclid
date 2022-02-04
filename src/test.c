#include <stdio.h>
#include <string.h>

#include "utils.h"

int main(int argc, char *argv[])
{
	char *z0 = "asd汉字";
	printf("strlen(z0) = %d\t\t// 0\n", strlen(z0));

	char *z0_c = str_clone(z0);
	printf("@@ strlen(z0_c) = %d\t\t// 0\n", strlen(z0_c));

	return 0;
}
