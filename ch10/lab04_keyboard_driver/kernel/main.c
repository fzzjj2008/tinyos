#include "print.h"
#include "init.h"
#include "interrupt.h"
// 测试keyboard消费者线程
#include "console.h"
#include "ioqueue.h"
#include "keyboard.h"

extern struct ioqueue keyboard_buffer;
void k_thread_a(void *args);
void k_thread_b(void *args);


int main(void) {
    put_str("kernel_init\n");
    init_all();
    thread_start("consumer_a", 31, k_thread_a, "A_");
    thread_start("consumer_b", 31, k_thread_b, "B_");
    intr_enable();
    while (1);
    return 0;
}

// 测试keyboard消费者线程
void k_thread_a(void *args) {
    while (1) {
        // 这里为了迁就ioq_putchar的ASSERT(intr_get_status() == INTR_OFF);
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&keyboard_buffer)) {
            console_put_str(args);
            char byte = ioq_getchar(&keyboard_buffer);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}

void k_thread_b(void *args) {
    while (1) {
        // 这里为了迁就ioq_putchar的ASSERT(intr_get_status() == INTR_OFF);
        enum intr_status old_status = intr_disable();
        if (!ioq_empty(&keyboard_buffer)) {
            console_put_str(args);
            char byte = ioq_getchar(&keyboard_buffer);
            console_put_char(byte);
        }
        intr_set_status(old_status);
    }
}
