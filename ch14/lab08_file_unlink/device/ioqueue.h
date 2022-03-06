#ifndef _DEVICE_IOQUEUE_H
#define _DEVICE_IOQUEUE_H

#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define bufsize 64

struct ioqueue {
    struct lock lock;
    struct task_struct* producer;
    struct task_struct* consumer;
    char buf[bufsize];
    int32_t head;
    int32_t tail;
};

bool ioq_full(struct ioqueue* queue);
void ioqueue_init(struct ioqueue* queue);
bool ioq_empty(struct ioqueue* queue);
char ioq_getchar(struct ioqueue* queue);
void ioq_putchar(struct ioqueue* queue, char byte);

#endif
