#include <stdio.h>
#include "asm.h"

void c_print(int ret)
{
    printf("The result is: %d\n", ret);
}

int main(void)
{
    int ret;
    // ret = c_sub(5, 3);
    ret = asm_sub(5, 3);
    printf("5 - 3 = %d\n", ret);
    return 0;
}
