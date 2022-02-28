#include "sync.h"
#include "interrupt.h"
#include "debug.h"

/**
 * 初始化信号量
 */
void semaphore_init(struct semaphore* psem, uint8_t value) {
    psem->value = value;                        // 信号量赋初值
    list_init(&psem->waiters);                  // 初始化信号量等待队列
}

/**
 * 初始化锁
 */
void lock_init(struct lock* lock) {
    lock->holder = NULL;
    lock->holder_repeat_num = 0;
    semaphore_init(&lock->semaphore, 1);        // 信号量初值为1
}

/**
 * 信号量down操作
 */
void semaphore_down(struct semaphore* psem) {
    // 关中断，保证原子操作
    enum intr_status old_status = intr_disable();

    // value为0表示锁被人持有
    // 注意这里是while不是if，原因是可以保证被唤醒后再次判断psem->value == 0
    while (psem->value == 0) {
        struct task_struct* cur = running_thread();
        ASSERT(!list_find(&psem->waiters, &cur->general_tag));
        // 当前线程不应该在信号量的waiters队列中
        if (list_find(&psem->waiters, &cur->general_tag)) {
            PANIC("semaphore_down: thread blocked has been in waiters_list\n");
        }
        // 当前线程加入锁的等待队列，阻塞自己
        list_append(&psem->waiters, &cur->general_tag);
        thread_block(TASK_BLOCKED);
    }

    // value为1或被唤醒后，获得锁，执行down操作
    psem->value--;
    ASSERT(psem->value == 0);
    // 恢复之前的中断状态
    intr_set_status(old_status);
}

/**
 * 信号量up操作
 */
void semaphore_up(struct semaphore* psem) {
    // 关中断，保证原子操作
    enum intr_status old_status = intr_disable();
    ASSERT(psem->value == 0);

    if (!list_empty(&psem->waiters)) {
        // 获取等待队列最前面的任务唤醒
        struct task_struct* waiter = elem2entry(struct task_struct, general_tag, list_pop(&psem->waiters));
        thread_unblock(waiter);
    }

    psem->value++;
    ASSERT(psem->value == 1);
    // 恢复之前的中断状态
    intr_set_status(old_status);
}

/**
 * 申请锁
 */
void lock_acquire(struct lock* plock) {
    struct task_struct* cur = running_thread();
    if (plock->holder != cur) {
        // 锁未持有，则持有者设为自己，down操作
        semaphore_down(&plock->semaphore);
        plock->holder = cur;
        ASSERT(plock->holder_repeat_num == 0);
        plock->holder_repeat_num = 1;
    } else {
        // 锁已持有，则holder_repeat_num+1
        plock->holder_repeat_num++;
    }
}

/**
 * 锁释放
 */
void lock_release(struct lock* plock) {
    ASSERT(plock->holder == running_thread());
    if (plock->holder_repeat_num > 1) {
        plock->holder_repeat_num--;
        return;
    }
    ASSERT(plock->holder_repeat_num == 1);
    // up操作
    plock->holder = NULL;
    plock->holder_repeat_num = 0;
    semaphore_up(&plock->semaphore);
}
