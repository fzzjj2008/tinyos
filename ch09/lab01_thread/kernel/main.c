#include "print.h"
#include "init.h"
#include "thread.h"

void k_thread_function(void*);

int main(void)
{
    put_str("kernel_init\n");
    init_all();

    thread_start("k_thread_a", 31, k_thread_function, "argA ");

    while (1);
    return 0;
}

void k_thread_function(void* args) {
    while (1) {
        put_str((char*) args);
    }
}
