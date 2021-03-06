#include "print.h"
#include "init.h"
#include "interrupt.h"
#include "process.h"
#include "console.h"
#include "syscall_init.h"
#include "syscall.h"
#include "stdio.h"
#include "memory.h"
#include "fs.h"

void k_thread_function_a(void* args);
void k_thread_function_b(void* args);
void user_process_a(void* args);
void user_process_b(void* args);
int prog_a_pid = 0, prog_b_pid = 0;

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();
    intr_enable();
    process_execute(user_process_a, "user_process_a");
    process_execute(user_process_b, "user_process_b");
    thread_start("k_thread_a", 31, k_thread_function_a, "threadA ");
    thread_start("k_thread_b", 31, k_thread_function_b, "threadB ");

    sys_open("/file1", O_CREAT);
    uint32_t fd = sys_open("/file1", O_RDONLY);
    printf("fd:%d\n", fd);
    sys_close(fd);
    printf("%d closed now\n", fd);
    while (1);
    return 0;
}

void k_thread_function_a(void* args) {
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str("thread_a malloc addr: 0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while (1);
}

void k_thread_function_b(void* args) {
    void* addr1 = sys_malloc(256);
    void* addr2 = sys_malloc(255);
    void* addr3 = sys_malloc(254);

    console_put_str("thread_b malloc addr: 0x");
    console_put_int((int)addr1);
    console_put_char(',');
    console_put_int((int)addr2);
    console_put_char(',');
    console_put_int((int)addr3);
    console_put_char('\n');

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    sys_free(addr1);
    sys_free(addr2);
    sys_free(addr3);
    while (1);
}

void user_process_a(void* args) {
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_a malloc addr: 0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2, (int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while (1);
}

void user_process_b(void* args) {
    void* addr1 = malloc(256);
    void* addr2 = malloc(255);
    void* addr3 = malloc(254);
    printf("prog_b malloc addr: 0x%x,0x%x,0x%x\n", (int)addr1, (int)addr2, (int)addr3);

    int cpu_delay = 100000;
    while(cpu_delay-- > 0);
    free(addr1);
    free(addr2);
    free(addr3);
    while (1);
}
