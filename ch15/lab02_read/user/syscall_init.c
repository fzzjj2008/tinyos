#include "syscall_init.h"
#include "syscall.h"
#include "thread.h"
#include "print.h"
#include "string.h"
#include "memory.h"
#include "fs.h"
#include "fork.h"

#define SYSCALL_CNT 32
typedef void* syscall;
syscall syscall_table[SYSCALL_CNT];


uint32_t sys_getpid(void) {
    return running_thread()->pid;
}

/**
 * 初始化系统调用
 */
void syscall_init(void) {
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE] = sys_write;
    syscall_table[SYS_MALLOC] = sys_malloc;
    syscall_table[SYS_FREE] = sys_free;
    syscall_table[SYS_FORK] = sys_fork;
    syscall_table[SYS_READ] = sys_read;
    syscall_table[SYS_PUTCHAR] = sys_putchar;
    syscall_table[SYS_CLEAR] = cls_screen;
    put_str("syscall_init done\n");
}
