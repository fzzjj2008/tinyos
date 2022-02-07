#ifndef _THREAD_SWITCH
#define _THREAD_SWITCH

void switch_to(struct task_struct* cur, struct task_struct* next);

#endif
