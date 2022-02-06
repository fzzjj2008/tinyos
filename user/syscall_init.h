#ifndef _USER_SYSCALL_INIT_H
#define _USER_SYSCALL_INIT_H

#include "stdint.h"

uint32_t sys_getpid(void);
void syscall_init(void);

#endif
