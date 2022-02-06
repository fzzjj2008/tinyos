#ifndef __LIB_STDINT_H
#define __LIB_STDINT_H

#include "stdint.h"

enum SYSCALL_NR {
    SYS_GETPID
}
uint32_t getpid(void);

#endif
