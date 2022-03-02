#include "ioqueue.h"
#include "interrupt.h"
#include "global.h"
#include "debug.h"
#include "sync.h"

/**
 * 初始化队列
 */
void ioqueue_init(struct ioqueue* queue) {
    lock_init(&queue->lock);
    queue->producer = queue->consumer = NULL;
    queue->head = queue->tail = 0;
}

/**
 * 返回pos在缓冲区中的下一个位置值
 */
static int32_t next_pos(int32_t pos) {
    return (pos + 1) % bufsize;
}

/**
 * 队列已满
 */
int is_queue_full(struct ioqueue* queue) {
    return next_pos(queue->head) == queue->tail;
}

/**
 * 队列已空
 */
int is_queue_empty(struct ioqueue* queue) {
    return queue->head == queue->tail;
}

/**
 * 生产者或消费者在此缓冲区上等待
 */
static void queue_wait(struct task_struct** waiter) {
    *waiter = running_thread();
    thread_block(TASK_BLOCKED);
}

/**
 * 唤醒等待的生产者或消费者
 */
static void wakeup(struct task_struct** waiter) {
    thread_unblock(*waiter);
    *waiter = NULL;
}

/**
 * 消费者从队列中获取一个字符，如果队列为空，那么等待
 */
char queue_getchar(struct ioqueue* queue) {
    ASSERT(intr_get_status() == INTR_OFF);

    while (is_queue_empty(queue)) {
        lock_acquire(&queue->lock);
        // 这里同时会把ioqueue的consumer置为当前线程
        queue_wait(&queue->consumer);
        lock_release(&queue->lock);
    }

    char byte = queue->buf[queue->tail];
    queue->tail = next_pos(queue->tail);

    if (queue->producer != NULL) {
        // 在无锁的情况下调用解除阻塞操作，因为是单核CPU且屏蔽了中断，所以是安全的
        wakeup(&queue->producer);
    }

    return byte;
}

/**
 * 生产者往队列写入一个字符
 */
void queue_putchar(struct ioqueue* queue, char byte) {
    ASSERT(intr_get_status() == INTR_OFF);

    while (is_queue_full(queue)) {
        lock_acquire(&queue->lock);
        queue_wait(&queue->producer);
        lock_release(&queue->lock);
    }

    queue->buf[queue->head] = byte;
    queue->head = next_pos(queue->head);

    if (queue->consumer != NULL) {
        wakeup(&queue->consumer);
    }
}
