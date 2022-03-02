#include "print.h"
#include "init.h"
#include "stdio.h"
#include "syscall.h"

void init(void);

int main(void) {
    // 这里不能使用console_put_str，因为还没有初始化
    put_str("kernel_init\n");
    init_all();
    while (1);
    return 0;
}

/**
 * init进程
 */
void init(void) {
   uint32_t ret_pid = fork();
   if(ret_pid) {
      printf("i am father, my pid is %d, child pid is %d\n", getpid(), ret_pid);
   } else {
      printf("i am child, my pid is %d, ret pid is %d\n", getpid(), ret_pid);
   }
   while(1);
}
