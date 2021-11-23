#include "print.h"
#include "init.h"
#include "interrupt.h"

int main(void)
{
    put_str("kernel_init\n");
    init_all();
    asm volatile ("int $0x00");
    asm volatile ("int $0x01");
    asm volatile ("int $0x02");
    asm volatile ("int $0x03");
    asm volatile ("int $0x04");
    asm volatile ("int $0x05");
    asm volatile ("int $0x06");
    asm volatile ("int $0x07");
    asm volatile ("int $0x08");
    asm volatile ("int $0x09");
    asm volatile ("int $0x0a");
    asm volatile ("int $0x0b");
    asm volatile ("int $0x0c");
    asm volatile ("int $0x0d");
    asm volatile ("int $0x0e");
    asm volatile ("int $0x0f");
    asm volatile ("int $0x10");
    asm volatile ("int $0x11");
    asm volatile ("int $0x12");
    asm volatile ("int $0x13");
    asm volatile ("int $0x14");
    asm volatile ("int $0x15");
    asm volatile ("int $0x16");
    asm volatile ("int $0x17");
    asm volatile ("int $0x18");
    asm volatile ("int $0x19");
    asm volatile ("int $0x1a");
    asm volatile ("int $0x1b");
    asm volatile ("int $0x1c");
    asm volatile ("int $0x1d");
    asm volatile ("int $0x1e");
    asm volatile ("int $0x1f");
    asm volatile ("int $0x20");
    while (1);
    return 0;
}
