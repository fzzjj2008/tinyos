#include "init.h"
#include "print.h"
#include "memory.h"
#include "interrupt.h"
#include "timer.h"

void init_all() {
    put_str("init_all.\n");
    idt_init();
    mem_init();
    timer_init();
}
