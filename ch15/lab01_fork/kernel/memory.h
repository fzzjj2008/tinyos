   
#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"
#include "list.h"

// 存在标志
#define PG_P_1 1
#define PG_P_0 0
// 只读
#define PG_RW_R 0
// 可写
#define PG_RW_W 2
// 系统级
#define PG_US_S 0
#define PG_US_U 4

// 内存池类型标志
enum pool_flags {
    PF_KERNEL = 1,                      // 内核内存池
    PF_USER = 2                         // 用户内存池
};

// 虚拟内存
struct virtual_addr {
    struct bitmap vaddr_bitmap;         // 虚拟内存位图，标志内存是否使用
    uint32_t vaddr_start;               // 虚拟内存的起始地址
};

// 内存块
struct mem_block {
    struct list_elem free_elem;
};

// 内存块描述符
struct mem_block_desc {
    uint32_t block_size;                // 内存块大小
    uint32_t blocks_per_arena;          // 本arena中可容纳内存块mem_block的数量
    struct list free_list;              // 目前可用的mem_block链表
};

#define MEM_BLOCK_DESC_CNT 7            // 内存块描述符总数(16, 32, ... , 1024字节)

extern struct pool kernel_pool, user_pool;

void block_desc_init(struct mem_block_desc* desc_array);
void mem_init(void);
void* get_kernel_pages(uint32_t page_count);
void* malloc_page(enum pool_flags pf, uint32_t page_count);
void mfree_page(enum pool_flags pf, void* _vaddr, uint32_t pg_cnt);
uint32_t addr_v2p(uint32_t vaddr);
void* get_a_page(enum pool_flags pf, uint32_t vaddr);
void* get_a_page_without_opvaddrbitmap(enum pool_flags pf, uint32_t vaddr);
void* get_user_pages(uint32_t page_count);
void* sys_malloc(uint32_t size);
void sys_free(void* ptr);

#endif
