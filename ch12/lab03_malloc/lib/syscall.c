#include "syscall.h"

#define _syscall0(NUMBER) ({                                    \
    int retval;                                                 \
    asm volatile (                                              \
        "int $0x80"                                             \
        : "=a" (retval)                                         \
        : "a" (NUMBER)                                          \
        : "memory"                                              \
    );                                                          \
    retval;                                                     \
})

#define _syscall1(NUMBER, ARG1) ({                              \
    int retval;                                                 \
    asm volatile (                                              \
        "int $0x80"                                             \
        : "=a" (retval)                                         \
        : "a" (NUMBER), "b" (ARG1)                              \
        : "memory"                                              \
    );                                                          \
    retval;                                                     \
})

#define _syscall2(NUMBER, ARG1, ARG2) ({                        \
    int retval;                                                 \
    asm volatile (                                              \
        "int $0x80"                                             \
        : "=a" (retval)                                         \
        : "a" (NUMBER), "b" (ARG1), "c" (ARG2)                  \
        : "memory"                                              \
    );                                                          \
    retval;                                                     \
})

#define _syscall3(NUMBER, ARG1, ARG2, ARG3) ({                  \
    int retval;                                                 \
    asm volatile (                                              \
        "int $0x80"                                             \
        : "=a" (retval)                                         \
        : "a" (NUMBER), "b" (ARG1), "c" (ARG2), "d" (ARG3)      \
        : "memory"                                              \
    );                                                          \
    retval;                                                     \
})

/**
 * ??????????????????pid
 */
uint32_t getpid() {
    return _syscall0(SYS_GETPID);
}

/**
 * ???????????????str
 */
uint32_t write(char* str) {
    return _syscall1(SYS_WRITE, str);
}

/**
 * ??????size???????????????????????????????????????
 */
void* malloc(uint32_t size) {
    return (void*)_syscall1(SYS_MALLOC, size);
}

/**
 * ??????ptr???????????????
 */
void free(void* ptr) {
    _syscall1(SYS_FREE, ptr);
}
