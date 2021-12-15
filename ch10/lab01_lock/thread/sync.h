#ifndef _THREAD_SYNC_H
#define _THREAD_SYNC_H

#include "list.h"
#include "thread.h"
#include "stdint.h"

/**
 * 信号量
 */
struct semaphore {
    uint8_t value;                          // 信号量初值
    struct list waiters;                    // 信号量等待队列
};

struct lock {
    struct task_struct* holder;             // 锁的持有者
    struct semaphore semaphore;             // 二元信号量实现锁
    uint32_t holder_repeat_num;             // 锁持有者重复申请锁的次数
};

void semaphore_init(struct semaphore* psem, uint8_t value);
void lock_init(struct lock* lock);
void semaphore_down(struct semaphore* psem);
void semaphore_up(struct semaphore* psem);
void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);

#endif
