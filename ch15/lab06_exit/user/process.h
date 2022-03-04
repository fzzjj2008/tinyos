#ifndef _USER_PROCESS_H
#define _USER_PROCESS_H

#include "thread.h"

#define USER_STACK3_VADDR (0xc0000000 - 0x1000)
#define USER_VADDR_START 0x8048000
#define DEFAULT_PRIO 31

void start_process(void* user_prog);
void page_dir_activate(struct task_struct* pthread);
void process_activate(struct task_struct* pthread);
void create_user_vaddr_bitmap(struct task_struct* user_process);
void process_execute(void* user_prog, char* name);
uint32_t* create_page_dir(void);

#endif
