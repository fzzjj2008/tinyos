#include "print.h"
#include "init.h"
#include "interrupt.h"

int main(void) {
    put_str("I am kernel.\n");
    init_all();

    intr_enable();

    while (1);
    return 0;
}
