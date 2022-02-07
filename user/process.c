#include "process.h"
#include "interrupt.h"
#include "memory.h"
#include "global.h"
#include "debug.h"
#include "tss.h"
#include "thread.h"
#include "string.h"

/**
 * 构建用户进程上下文信息
 */
void start_process(void* user_prog) {
    void* function = user_prog;
    struct task_struct* cur = running_thread();

    // 指向中断栈(intr_stack的低端，即低地址处)，PCB布局回顾(从上到下表示内存地址从高到低)
    // 1. intr_stack
    // 2. thread_stack
    // PCB的属性self_kstack在线程创建完毕后指向thread_stack的最底部
    cur->self_kstack += sizeof(struct thread_stack);
    struct intr_stack* proc_stack = (struct intr_stack*) cur->self_kstack;

    proc_stack->edi = proc_stack->esi = proc_stack->ebp = proc_stack->esp_dummy = 0;
    proc_stack->ebx = proc_stack->edx = proc_stack->ecx = proc_stack->eax = proc_stack->gs = 0;

    // 为通过中断返回的方式进入3特权级做准备
    proc_stack->ds = proc_stack->es = proc_stack->fs = SELECTOR_U_DATA;
    proc_stack->eip = function;
    proc_stack->cs = SELECTOR_U_CODE;
    proc_stack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    proc_stack->esp = (void*) ((uint32_t) get_a_page(PF_USER, USER_STACK3_VADDR) + PAGE_SIZE);
    proc_stack->ss = SELECTOR_U_DATA;

    // 调用iret进入3特权级
    asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (proc_stack) : "memory");
}

/**
 * 如果给定的PCB是进程，那么将其页表设置到CR3
 */
void page_dir_activate(struct task_struct* pthread) {
    // 内核页表的物理地址，定义在boot.h，进入保护模式时确定
    uint32_t pagedir_phy_addr = 0x100000;

    if (pthread->pgdir != NULL) {
        // 用户进程，得到其页表地址
        pagedir_phy_addr = addr_v2p((uint32_t) pthread->pgdir);
    }

    // 更新页目录寄存器CR3，使新页表生效
    asm volatile ("movl %0, %%cr3" : : "r" (pagedir_phy_addr) : "memory");
}

/**
 * 激活线程或进程的页表，更新tss中的esp0为进程的0特权级栈
 */
void process_activate(struct task_struct* pthread) {
    ASSERT(pthread != NULL);

    page_dir_activate(pthread);

    // 内核线程特权级是0，处理器中断不会从tss获取0特权级栈地址，不需要更新esp0
    if (pthread->pgdir != NULL) {
        // 更新该用户进程的esp0，用于此进程被中断时保留上下文
        update_tss_esp(pthread);
    }
}

/**
 * 为进程创建页目录表
 */
uint32_t* create_page_dir(void) {
    uint32_t* page_dir_vaddr = get_kernel_pages(1);
    if (page_dir_vaddr == NULL) {
        PANIC("create_page_dir: get_kernel_pages failed!");
        return NULL;
    }

    // 用户进程最高1G指向Linux的内核空间，内核共享
    // 原理：二级页表可定义4GB空间。页目录表共1024项，0~767是用户进程页目录项；768~1023是内核的页目录项
    // 只要想办法将将内核的页表复制到进程页目录项中，可实现内核的共享
    // 其中768*4表示第768个页目录项的偏移量，4是页目录项大小；0xfffff000是页目录表基地址；1024表示要复制1024/4=256个页目录项
    // 参见图11-20
    memcpy((uint32_t*) ((uint32_t) page_dir_vaddr + 768 * 4), (uint32_t*) (0xfffff000 + 768 * 4), 1024);

    // 设置最后一项页表的地址为页目录地址
    uint32_t new_page_dir_phy_addr = addr_v2p((uint32_t) page_dir_vaddr);
    page_dir_vaddr[1023] = (new_page_dir_phy_addr | PG_US_U | PG_RW_W | PG_P_1);
    
    return page_dir_vaddr;
}

/**
 * 为用户进程设置其单独的虚拟地址池
 */
void create_user_vaddr_bitmap(struct task_struct* user_process) {
    user_process->userprog_vaddr.vaddr_start = USER_VADDR_START;
    // 0xc0000000是内核虚拟地址起始处
    // 计算位图所需要的内存页框数，DIV_ROUND_UP是除法向上取整
    uint32_t bitmap_page_count = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8, PAGE_SIZE);

    // 为用户进程位图分配内存
    user_process->userprog_vaddr.vaddr_bitmap.bits = get_kernel_pages(bitmap_page_count);
    user_process->userprog_vaddr.vaddr_bitmap.btmp_bytes_len = (0xc0000000 - USER_VADDR_START) / PAGE_SIZE / 8;

    bitmap_init(&user_process->userprog_vaddr.vaddr_bitmap);
}

/**
 * 创建用户进程
 */
void process_execute(void* user_prog, char* name) {
    // 内核内存池申请一个PCB数据结构，内核维护进程信息
    struct task_struct* thread = get_kernel_pages(1);
    
    init_thread(thread, name, DEFAULT_PRIO);
    create_user_vaddr_bitmap(thread);
    thread_create(thread, start_process, user_prog);
    thread->pgdir = create_page_dir();
    block_desc_init(thread->u_block_desc);

    enum intr_status old_status = intr_disable();
    ASSERT(!list_find(&thread_ready_list, &thread->general_tag));
    list_append(&thread_ready_list, &thread->general_tag);

    ASSERT(!list_find(&thread_all_list, &thread->all_list_tag));
    list_append(&thread_all_list, &thread->all_list_tag);
    intr_set_status(old_status);
}
