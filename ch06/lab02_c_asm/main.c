#include <stdio.h>

int c_sub(int a, int b)
{
    return a - b;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    asm_hello();
    ret = asm_sub(1, 10);
    printf("The return value is %d.\n", ret);
    return 0;
}
