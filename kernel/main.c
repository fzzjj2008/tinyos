#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"
#include "syscall_init.h"
#include "syscall.h"

void k_thread_function_a(void* args);
void k_thread_function_b(void* args);
void user_process_a(void* args);
void user_process_b(void* args);
int prog_a_pid = 0, prog_b_pid = 0;

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();

    process_execute(user_process_a, "user_process_a");
    process_execute(user_process_b, "user_process_b");

    intr_enable();
    console_put_str("main_pid: 0x");
    console_put_int(sys_getpid());
    console_put_char('\n');
    thread_start("k_thread_a", 31, k_thread_function_a, "threadA ");
    thread_start("k_thread_b", 31, k_thread_function_b, "threadB ");

    while (1);
    return 0;
}

void k_thread_function_a(void* args) {
    console_put_str("thread_a_pid: 0x");
    console_put_int(sys_getpid());
    console_put_char('\n');
    console_put_str("prog_a_pid: 0x");
    console_put_int(prog_a_pid);
    console_put_char('\n');
    while (1);
}

void k_thread_function_b(void* args) {
    console_put_str("thread_b_pid: 0x");
    console_put_int(sys_getpid());
    console_put_char('\n');
    console_put_str("prog_b_pid: 0x");
    console_put_int(prog_b_pid);
    console_put_char('\n');
    while (1);
}

void user_process_a(void* args) {
    prog_a_pid = getpid();
    while (1);
}

void user_process_b(void* args) {
    prog_b_pid = getpid();
    while (1);
}
