#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "debug.h"

int main(void)
{
    put_str("kernel_init\n");
    init_all();
    // ASSERT(1 == 1);
    ASSERT(1 == 2);
    while (1);
    return 0;
}
