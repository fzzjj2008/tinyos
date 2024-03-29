#include "timer.h"
#include "io.h"
#include "print.h"
#include "thread.h"
#include "debug.h"
#include "interrupt.h"

// 计数器0设为100Hz
#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

#define mil_seconds_per_intr (1000 / IRQ0_FREQUENCY)

/**
 * 内核自开启中断后所有的嘀嗒数.
 */
uint32_t ticks;

static void frequency_set(uint8_t counter_port,
                          uint8_t counter_no,
                          uint8_t rwl,
                          uint8_t counter_mode,
                          uint16_t counter_value) {
    outb(PIT_CONTROL_PORT, (uint8_t) (counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t) counter_value);
    outb(counter_port, (uint8_t) counter_value >> 8);
}

static void intr_timer_handler(void) {
    struct task_struct* cur_thread = running_thread();

    ASSERT(cur_thread->stack_magic == 0x77777777);

    cur_thread->elapsed_ticks++;
    ticks++;

    if (cur_thread->ticks == 0) {
        schedule();
    } else {
        cur_thread->ticks--;
    }
}

// 以tick为单位的sleep,任何时间形式的sleep会转换此ticks形式
static void ticks_to_sleep(uint32_t sleep_ticks) {
    uint32_t start_tick = ticks;
    while (ticks - start_tick < sleep_ticks) {     // 若间隔的ticks数不够便让出cpu
        thread_yield();
    }
}

// 以毫秒为单位的sleep   1秒= 1000毫秒
void mtime_sleep(uint32_t m_seconds) {
    uint32_t sleep_ticks = DIV_ROUND_UP(m_seconds, mil_seconds_per_intr);
    ASSERT(sleep_ticks > 0);
    ticks_to_sleep(sleep_ticks); 
}

/**
 * 初始化PIT 8253
 */
void timer_init() {
    put_str("timer_init start\n");
    // 控制字：计数器0, 读写方式先读高字节再读低字节, 工作方式2, 计数器初值COUNTER0_VALUE
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    register_handler(0x20, intr_timer_handler);
    put_str("timer_init done\n");
}
