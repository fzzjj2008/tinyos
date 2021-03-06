#include "thread.h"
#include "string.h"
#include "global.h"
#include "memory.h"
#include "interrupt.h"
#include "debug.h"
#include "print.h"
#include "process.h"
#include "sync.h"

struct task_struct* main_thread;            // 主线程PCB
struct task_struct* idle_thread;            // idle线程
struct list thread_ready_list;              // 就绪队列
struct list thread_all_list;                // 所有任务队列
static struct list_elem* thread_tag;        // 用于保存队列中的线程结点
struct lock pid_lock;                       // 分配pid使用的lock

/**
 * 任务切换
 */
extern void switch_to(struct task_struct* cur, struct task_struct* next);

static void kernel_thread(thread_func* function, void* func_args) {
    // 进入中断处理后会关闭中断，因此执行function前要开中断，避免时钟中断被屏蔽无法切换
    intr_enable();
    function(func_args);
}

/* 系统空闲时运行的线程 */
static void idle(void* arg UNUSED) {
   while(1) {
      thread_block(TASK_BLOCKED);     
      //执行hlt时必须要保证目前处在开中断的情况下
      asm volatile ("sti; hlt" : : : "memory");
   }
}

/**
 * 获取当前栈指针的高20位，作为线程PCB地址
 */
struct task_struct* running_thread() {
    uint32_t esp;
    asm ("mov %%esp, %0" : "=g" (esp));
    return (struct task_struct*) (esp & 0xfffff000);
}

/**
 * 分配pid
 */
static pid_t allocate_pid(void) {
    static pid_t next_pid =0;
    lock_acquire(&pid_lock);
    next_pid++;
    lock_release(&pid_lock);
    return next_pid;
}

/**
 * 初始化线程栈
 */
void thread_create(struct task_struct* pthread, thread_func function, void* func_args) {
    // 预留中断和线程使用栈的空间
    pthread->self_kstack -= sizeof(struct intr_stack);
    pthread->self_kstack -= sizeof(struct thread_stack);

    struct thread_stack* kthread_stack = (struct thread_stack*) pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_args = func_args;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->edi = kthread_stack->esi = 0;
}

/**
 * 初始化线程基本信息
 */
void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    pthread->pid = allocate_pid();
    strcpy(pthread->name, name);

    if (pthread == main_thread) {
        pthread->status = TASK_RUNNING;
    } else {
        pthread->status = TASK_READY;
    }

    pthread->priority = prio;
    pthread->ticks = prio;
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;                      // 线程没有自己的地址空间，置空
    // PCB所在物理页的顶端地址
    pthread->self_kstack = (uint32_t*) ((uint32_t) pthread + PAGE_SIZE);
    /* 预留标准输入输出 */
    pthread->fd_table[0] = 0;
    pthread->fd_table[1] = 1;
    pthread->fd_table[2] = 2;
    /* 其余的全置为-1 */
    uint8_t fd_idx = 3;
    while (fd_idx < MAX_FILES_OPEN_PER_PROC) {
        pthread->fd_table[fd_idx] = -1;
        fd_idx++;
    }
    pthread->cwd_inode_nr = 0;                  // 以根目录为默认工作路径
    pthread->stack_magic = 0x77777777;          // 自定义的魔数
}

/**
 * 创建线程
 */
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_args) {
    struct task_struct* thread = get_kernel_pages(1);

    init_thread(thread, name, prio);
    thread_create(thread, function, func_args);

    // 加入就绪线程队列
    ASSERT(!list_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    // 加入全部线程队列
    ASSERT(!list_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    return thread;
}

/**
 * 线程调度
 */
void schedule() {
    ASSERT(intr_get_status() == INTR_OFF);

    struct task_struct* cur_thread = running_thread();
    if (cur_thread->status == TASK_RUNNING) {
        // 时间片用完，当前线程加到就绪队列尾
        ASSERT(!list_find(&thread_ready_list, &cur_thread->general_tag));
        list_append(&thread_ready_list, &cur_thread->general_tag);
        cur_thread->ticks = cur_thread->priority;
        cur_thread->status = TASK_READY;
    }

    /* 如果就绪队列中没有可运行的任务,就唤醒idle */
    if (list_empty(&thread_ready_list)) {
        thread_unblock(idle_thread);
    }

    // 当前没有实现idle线程，所以要保证必须有可调度的线程存在
    ASSERT(!list_empty(&thread_ready_list));

    thread_tag = list_pop(&thread_ready_list);
    struct task_struct* next = elem2entry(struct task_struct, general_tag, thread_tag);
    next->status = TASK_RUNNING;

    // 激活任务页表，任务是用户进程则还要修改tss的esp0
    process_activate(next);
    
    switch_to(cur_thread, next);
}

/**
 * 阻塞当前线程
 */
void thread_block(enum task_status status) {
    ASSERT(status == TASK_BLOCKED || status == TASK_HANGING || status == TASK_WAITING);

    enum intr_status old_status = intr_disable();

    struct task_struct* cur = running_thread();
    cur->status = status;
    schedule();

    // 等到当前线程再次被调度时才能执行下面的语句
    // 调度的其它线程无非两种情况:
    // 1. 如果第一次执行，那么在kernel_thread方法中第一件事就是开中断
    // 2. 如果不是第一次执行，那么通过中断返回的方式继续执行，而iret执行也会再次开中断
    intr_set_status(old_status);
}

void thread_unblock(struct task_struct* pthread) {
    enum intr_status old_status = intr_disable();

    ASSERT(pthread->status == TASK_BLOCKED || pthread->status == TASK_HANGING || pthread->status == TASK_WAITING);

    // 如果pthread线程状态不是READY，将其放到就绪队列最前面，可尽快调度
    if (pthread->status != TASK_READY) {
        ASSERT(!list_find(&thread_ready_list, &pthread->general_tag));
        list_push(&thread_ready_list, &pthread->general_tag);
        pthread->status = TASK_READY;
    }

    intr_set_status(old_status);
}

static void make_main_thread() {
    main_thread = running_thread();
    init_thread(main_thread, "main", 31);

    // main线程正在运行，故无需加到ready队列
    ASSERT(!list_find(&thread_all_list, &main_thread->all_list_tag));
    list_append(&thread_all_list, &main_thread->all_list_tag);
}

/* 主动让出cpu,换其它线程运行 */
void thread_yield(void) {
    struct task_struct* cur = running_thread();   
    enum intr_status old_status = intr_disable();
    ASSERT(!list_find(&thread_ready_list, &cur->general_tag));
    list_append(&thread_ready_list, &cur->general_tag);
    cur->status = TASK_READY;
    schedule();
    intr_set_status(old_status);
}

/**
 * 线程模块初始化
 */
void thread_init() {
    put_str("thread_init start\n");
    list_init(&thread_all_list);
    list_init(&thread_ready_list);
    lock_init(&pid_lock);
    make_main_thread();
    /* 创建idle线程 */
    idle_thread = thread_start("idle", 10, idle, NULL);
    put_str("thread_init done\n");
}
