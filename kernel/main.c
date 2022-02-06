#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"

void k_thread_function_a(void* args);
void k_thread_function_b(void* args);
void user_process_a(void* args);
void user_process_b(void* args);
int test_var_a = 0, test_var_b = 0;

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();

    thread_start("k_thread_a", default_prio, k_thread_function_a, "threadA ");
    thread_start("k_thread_b", default_prio, k_thread_function_b, "threadB ");
    process_execute(user_process_a, "user_process_a");
    process_execute(user_process_b, "user_process_b");

    intr_enable();

    while (1);
    return 0;
}

void k_thread_function_a(void* args) {
    while (1) {
        console_put_str("v_a: 0x");
        console_put_int(test_var_a);
    }
}

void k_thread_function_b(void* args) {
    while (1) {
        console_put_str("v_b: 0x");
        console_put_int(test_var_b);
    }
}

void user_process_a(void* args) {
    while (1) {
        test_var_a++;
    }
}

void user_process_b(void* args) {
    while (1) {
        test_var_b++;
    }
}
