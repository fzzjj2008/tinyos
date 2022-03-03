#include "print.h"
#include "init.h"
#include "console.h"
#include "stdio.h"
#include "syscall.h"
#include "debug.h"
#include "shell.h"

void init(void);

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();
    cls_screen();
    console_put_str("[rabbit@localhost /]$ ");
    while (1);
    return 0;
}

/**
 * init进程
 */
void init(void) {
   uint32_t ret_pid = fork();
   if(ret_pid) {  // 父进程
      while(1);
   } else {	  // 子进程
      my_shell();
   }
   PANIC("init: should not be here");
}
