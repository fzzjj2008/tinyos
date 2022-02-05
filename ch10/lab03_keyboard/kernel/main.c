#include "print.h"
#include "init.h"
#include "interrupt.h"

int main(void) {
    put_str("kernel_init\n");
    init_all();

    intr_enable();

    while (1);
    return 0;
}
