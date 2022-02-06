#include "syscall_init.h"
#include "stdint.h"

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
    put_str("syscall_init done\n");
}