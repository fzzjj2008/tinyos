#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "debug.h"
#include "memory.h"

int main(void)
{
    put_str("kernel_init\n");
    init_all();

    void* vaddr = get_kernel_pages(3);
    put_str("\nstart vaddr: ");
    put_int((uint32_t) vaddr);
    put_char('\n');

    while (1);
    return 0;
}
