#ifndef _THREAD_H
#define _THREAD_H

#include "stdint.h"
#include "list.h"
#include "memory.h"

typedef void thread_func(void*);
typedef int16_t pid_t;

// 线程状态
enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DEAD
};

// 中断栈，中断时保存上下文
struct intr_stack {
    uint32_t vec_no;
    uint32_t edi;  
    uint32_t esi;  
    uint32_t ebp;  
    uint32_t esp_dummy;  
    uint32_t ebx;  
    uint32_t edx;  
    uint32_t ecx;  
    uint32_t eax;  
    uint32_t gs;  
    uint32_t fs;  
    uint32_t es;  
    uint32_t ds;

    // 下面的属性由CPU从低特权级进入高特权级时压入
    uint32_t err_code;  
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

// 线程栈，eip保存待调用的函数地址或切换后新任务的返回地址
struct thread_stack {
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    // 线程第一次执行时eip指向待调用的函数kernel_thread，其它时候指向switch_to的返回地址
    void (*eip) (thread_func* func, void* func_args);

    // 以下仅供第一次被调度上CPU时使用
    void (*unused_retaddr);         // 占4字节位置表示返回地址
    thread_func* function;          // kernel_thread所调用的函数名
    void* func_args;                // kernel_thread所调用的函数的参数
};

// PCB，进程或线程的控制块
struct task_struct {
    uint32_t* self_kstack;          // 内核栈
    pid_t pid;                      // 进程PID
    enum task_status status;        // 线程状态
    uint8_t priority;               // 线程优先级
    char name[16];                  // 线程名称
    uint8_t ticks;                  // 任务时间片
    uint32_t elapsed_ticks;         // 任务运行至结束的总时钟数
    struct list_elem general_tag;   // 可执行队列节点
    struct list_elem all_list_tag;  // 线程被加入全部线程队列使用
    uint32_t* pgdir;                // 进程自己页表的虚拟地址
    struct virtual_addr userprog_vaddr;  // 用户进程的虚拟地址
    struct mem_block_desc u_block_desc[MEM_BLOCK_DESC_CNT];     // 用户进程内存块描述符
    uint32_t stack_magic;           // 魔数，栈的边界标记，检测栈的溢出，防止压栈时PCB被覆盖
};

struct task_struct* main_thread;
struct list thread_ready_list;
struct list thread_all_list;

struct task_struct* running_thread();
void thread_create(struct task_struct* pthread, thread_func function, void* func_args);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_args);
void schedule();
void thread_init();
void thread_block(enum task_status status);
void thread_unblock(struct task_struct* pthread);

#endif
