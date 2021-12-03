#include "print.h"
#include "init.h"
#include "thread.h"
#include "interrupt.h"

void k_thread_function_a(void*);
void k_thread_function_b(void*);

int main(void) {
    put_str("kernel_init\n");
    init_all();

    thread_start("k_thread_a", 31, k_thread_function_a, "threadA ");
    thread_start("k_thread_b", 8, k_thread_function_b, "threadB ");

    intr_enable();

    while (1) {
        put_str("main ");
    }

    return 0;
}

void k_thread_function_a(void* args) {
    while (1) {
        put_str((char*) args);
    }
}

void k_thread_function_b(void* args) {
    while (1) {
        put_str((char*) args);
    }
}
