#include "syscall.h"
#include "thread.h"

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
uint32_t write(int32_t fd, const void* buf, uint32_t count) {
    return _syscall3(SYS_WRITE, fd, buf, count);
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

/* ???????????????,???????????????pid */
pid_t fork(void){
   return _syscall0(SYS_FORK);
}

/* ??????????????????fd?????????count????????????buf */
int32_t read(int32_t fd, void* buf, uint32_t count) {
   return _syscall3(SYS_READ, fd, buf, count);
}

/* ?????????????????? */
void putchar(char char_asci) {
   _syscall1(SYS_PUTCHAR, char_asci);
}

/* ???????????? */
void clear(void) {
   _syscall0(SYS_CLEAR);
}

/* ???????????????????????? */
char* getcwd(char* buf, uint32_t size) {
   return (char*)_syscall2(SYS_GETCWD, buf, size);
}

/* ???flag??????????????????pathname */
int32_t open(char* pathname, uint8_t flag) {
   return _syscall2(SYS_OPEN, pathname, flag);
}

/* ????????????fd */
int32_t close(int32_t fd) {
   return _syscall1(SYS_CLOSE, fd);
}

/* ????????????????????? */
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence) {
   return _syscall3(SYS_LSEEK, fd, offset, whence);
}

/* ????????????pathname */
int32_t unlink(const char* pathname) {
   return _syscall1(SYS_UNLINK, pathname);
}

/* ????????????pathname */
int32_t mkdir(const char* pathname) {
   return _syscall1(SYS_MKDIR, pathname);
}

/* ????????????name */
struct dir* opendir(const char* name) {
   return (struct dir*)_syscall1(SYS_OPENDIR, name);
}

/* ????????????dir */
int32_t closedir(struct dir* dir) {
   return _syscall1(SYS_CLOSEDIR, dir);
}

/* ????????????pathname */
int32_t rmdir(const char* pathname) {
   return _syscall1(SYS_RMDIR, pathname);
}

/* ????????????dir */
struct dir_entry* readdir(struct dir* dir) {
   return (struct dir_entry*)_syscall1(SYS_READDIR, dir);
}

/* ?????????????????? */
void rewinddir(struct dir* dir) {
   _syscall1(SYS_REWINDDIR, dir);
}

/* ??????path?????????buf??? */
int32_t stat(const char* path, struct stat* buf) {
   return _syscall2(SYS_STAT, path, buf);
}

/* ?????????????????????path */
int32_t chdir(const char* path) {
   return _syscall1(SYS_CHDIR, path);
}

/* ?????????????????? */
void ps(void) {
   _syscall0(SYS_PS);
}
