#include "timer.h"
#include "io.h"
#include "print.h"

// 计数器0设为100Hz
#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43


static void frequency_set(uint8_t counter_port,
                          uint8_t counter_no,
                          uint8_t rwl,
                          uint8_t counter_mode,
                          uint16_t counter_value) {
    outb(PIT_CONTROL_PORT, (uint8_t) (counter_no << 6 | rwl << 4 | counter_mode << 1));
    outb(counter_port, (uint8_t) counter_value);
    outb(counter_port, (uint8_t) counter_value >> 8);
}

/**
 * 初始化PIT 8253
 */
void timer_init() {
    put_str("timer_init start.\n");
    // 控制字：计数器0, 读写方式先读高字节再读低字节, 工作方式2, 计数器初值COUNTER0_VALUE
    frequency_set(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    put_str("timer_init done.\n");
}
