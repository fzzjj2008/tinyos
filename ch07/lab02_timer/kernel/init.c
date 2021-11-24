#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"

void init_all() {
    put_str("init_all.\n");
    idt_init();
    timer_init();               // 初始化timer，这句话注释掉可以看到中断频率没有上来
}
