#include "memory.h"
#include "stdint.h"
#include "print.h"
#include "string.h"
#include "debug.h"
#include "sync.h"
#include "global.h"
#include "interrupt.h"

// 位图地址范围：0xc009a000 ~ 0xc009dfff
// tinyos总共最多支持512MB内存，对应位图大小512MB / 4KB=131072 bit=16KB
#define MEM_BITMAP_BASE 0xc009a000
// 内核堆空间虚拟地址：0xc0100000
// 跨过1M内存，使虚拟地址在逻辑上连续
#define K_HEAP_START 0xc0100000
// 获取高10位页目录项地址
#define PDE_INDEX(addr) ((addr & 0xffc00000) >> 22)
// 获取中间10位页表地址
#define PTE_INDEX(addr) ((addr & 0x003ff000) >> 12)
 
// 内存池结构体
struct pool {
    struct bitmap pool_bitmap;          // 内存池位图
    uint32_t phy_addr_start;            // 内存池物理起始地址
    uint32_t pool_size;                 // 内存池字节容量
    struct lock lock;                   // 申请内存时互斥
};

// 内存仓库
struct arena {
    struct mem_block_desc* desc;        // 此arena关联的内存块描述符
    uint32_t cnt;                       // large=true表示arena占用的页框数，否则表示arena中还有多少空闲内存(空闲mem_block数量)
    bool large;                         // 
};

struct pool kernel_pool, user_pool;     // 内核内存池和用户内存池
struct virtual_addr kernel_vaddr;       // 虚拟地址池，用来给内核分配虚拟地址
struct mem_block_desc k_block_descs[MEM_BLOCK_DESC_CNT];    //内核内存块描述符数组

/**
 * 初始化内存池
 */
static void mem_pool_init(uint32_t all_mem) {
    put_str("mem_pool_init start\n");

    // 一级和二级页表大小：页目录表+页表大小，总共(1+1+254)*4KB=1MB
    //   页目录表：1张页目录表，大小4KB
    //     PDE0, PDE768：指向页表1，即物理地址透传
    //     PDE1023：指向页目录表自身
    //     PDE769~1022：对应254张内核的页表
    //   页表：每张页表大小4KB
    uint32_t page_table_size = PAGE_SIZE * 256;

    // 已经使用的内存为: 低端1MB内存 + 现有的页表和页目录占据的空间1MB
    uint32_t used_mem = (page_table_size + 0x100000);

    uint32_t free_mem = (all_mem - used_mem);
    uint16_t free_pages = free_mem / PAGE_SIZE;

    // 可用的物理内存分成两半，一半是内核空间，一半是用户空间
    uint16_t kernel_free_pages = free_pages / 2;
    uint16_t user_free_pages = (free_pages - kernel_free_pages);

    // 内核空间bitmap长度(字节)，每一位代表一页
    uint32_t kernel_bitmap_length = kernel_free_pages / 8;
    uint32_t user_bitmap_length = user_free_pages / 8;

    // 内核内存池起始物理地址，注意内核的虚拟地址占据地址空间的顶端，但是实际映射的物理地址是在这里
    uint32_t kernel_pool_start = used_mem;
    uint32_t user_pool_start = (kernel_pool_start + kernel_free_pages * PAGE_SIZE);

    kernel_pool.phy_addr_start = kernel_pool_start;
    user_pool.phy_addr_start = user_pool_start;

    kernel_pool.pool_size = kernel_free_pages * PAGE_SIZE;
    user_pool.pool_size = user_free_pages * PAGE_SIZE;

    kernel_pool.pool_bitmap.btmp_bytes_len = kernel_bitmap_length;
    user_pool.pool_bitmap.btmp_bytes_len = user_bitmap_length;

    // 内核bitmap和user bitmap bit数组的起始地址
    kernel_pool.pool_bitmap.bits = (void*) MEM_BITMAP_BASE;
    user_pool.pool_bitmap.bits = (void*) (MEM_BITMAP_BASE + kernel_bitmap_length);

    bitmap_init(&kernel_pool.pool_bitmap);
    bitmap_init(&user_pool.pool_bitmap);

    // 内核虚拟地址池，紧挨在内核内存池和用户内存池后
    kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kernel_bitmap_length;
    kernel_vaddr.vaddr_bitmap.bits = (void*) (MEM_BITMAP_BASE + kernel_bitmap_length + user_bitmap_length);
    kernel_vaddr.vaddr_start = K_HEAP_START;
    bitmap_init(&kernel_vaddr.vaddr_bitmap);

    // 打印信息
    put_str("kernel pool bitmap address: ");
    put_int((uint32_t) kernel_pool.pool_bitmap.bits);
    put_char('\n');

    put_str("kernel pool physical address: ");
    put_int(kernel_pool.phy_addr_start);
    put_char('\n');
    
    put_str("user pool bitmap address: ");
    put_int((uint32_t) user_pool.pool_bitmap.bits);
    put_char('\n');
    put_str("user pool physical address: ");
    put_int(user_pool.phy_addr_start);
    put_char('\n');
    
    put_str("kernel vaddr bitmap address: ");
    put_int((uint32_t) kernel_vaddr.vaddr_bitmap.bits);
    put_char('\n');
    put_str("kernel vaddr bitmap len: ");
    put_int(kernel_vaddr.vaddr_bitmap.btmp_bytes_len);
    put_char('\n');

    put_str("mem_pool_init done\n");
}

/**
 * 申请pg_count个的虚拟页
 * Return: 虚拟页的起始地址，失败返回NULL
 */
static void* vaddr_get(enum pool_flags pf, uint32_t pg_count) {
    int vaddr_start = 0, bit_idx_start = -1;
    uint32_t count = 0;

    if (pf == PF_KERNEL) {
        // 内核内存池，从bitmap分配
        bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_count);
        if (bit_idx_start == -1) {
            // 申请失败，虚拟内存不足
            return NULL;
        }

        // 修改bitmap，占用虚拟内存
        while (count < pg_count) {
            bitmap_set(&kernel_vaddr.vaddr_bitmap, (bit_idx_start + count), 1);
            ++count;
        }

        vaddr_start = (kernel_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE);

    } else {
        // 用户内存池
        struct task_struct* cur = running_thread();
        bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_count);
        if (bit_idx_start == -1) {
            // 申请失败，虚拟内存不足
            return NULL;
        }

        while (count < pg_count) {
            bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + count, 1);
            ++count;
        }

        vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PAGE_SIZE;
        // 0xc0000000 - PAGE_SIZE作为用户3级栈已经在start_process被分配
        ASSERT((uint32_t)vaddr_start < (0xc0000000 - PAGE_SIZE));
    }

    return (void*) vaddr_start;
}

/**
 * 构造能够访问vaddr所在pte的虚拟地址
 */
static uint32_t* pte_ptr(uint32_t vaddr) {
    // 1. 访问高10位，即PDE1023，根据loader.S的页目录表即访问自身
    // 2. 访问中10位，即vaddr的高10位
    // 3. 访问低10位，即vaddr的中10位。*4是因为页表每项是4字节 (参考图5-14)
    return (uint32_t*) (0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (PTE_INDEX(vaddr) * 4));
}

/**
 * 构造能够访问vaddr所在pde的虚拟地址
 */
static uint32_t* pde_ptr(uint32_t vaddr) {
    // 访问PDE1023+PTE1023, 根据loader.S的页目录表即访问自身
    return (uint32_t*) ((0xfffff000) + (PDE_INDEX(vaddr) * 4));
}

/**
 * 在给定的物理内存池中分配一个物理页，返回其物理地址
 */
static void* palloc(struct pool* m_pool) {
    // m_pool->pool_bitmap申请一页内存
    int bit_index = bitmap_scan(&m_pool->pool_bitmap, 1);
    if (bit_index == -1) {
        return NULL;
    }
    bitmap_set(&m_pool->pool_bitmap, bit_index, 1);
    uint32_t page_phyaddr = ((bit_index * PAGE_SIZE) + m_pool->phy_addr_start);
    return (void*) page_phyaddr;
}

/**
 * 通过页表建立虚拟页与物理页的映射关系
 */
static void page_table_add(void* _vaddr, void* _page_phyaddr) {
    uint32_t vaddr = (uint32_t) _vaddr, page_phyaddr = (uint32_t) _page_phyaddr;
    uint32_t* pde = pde_ptr(vaddr);
    uint32_t* pte = pte_ptr(vaddr);

    if (*pde & PG_P_1) {
        // 页目录项已经存在
        if (!(*pte & PG_P_1)) {
            // 页表项不存在，构造页表项
            *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        }
    } else {
        // 页目录项不存在，从内核池分配一个物理页作为页表
        uint32_t pde_phyaddr = (uint32_t) palloc(&kernel_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        // 清理物理页
        memset((void*) ((int) pte & 0xfffff000), 0, PAGE_SIZE);
        *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
}

/**
 * 分配page_count个页空间，自动建立虚拟页与物理页的映射
 */
void* malloc_page(enum pool_flags pf, uint32_t page_count) {
    // 内核和用户空间是15MB，所以page_count是0~3840 (P.S.魔数)
    ASSERT(page_count > 0 && page_count < 3840);

    // 1. 在虚拟地址池中申请虚拟内存
    void* vaddr_start = vaddr_get(pf, page_count);
    if (vaddr_start == NULL) {
        return NULL;
    }

    uint32_t vaddr = (uint32_t) vaddr_start, count = page_count;
    struct pool* mem_pool = (pf & PF_KERNEL) ? &kernel_pool : &user_pool;

    while (count > 0) {
        // 2. 物理内存池申请物理页，物理页可以不连续，逐个映射
        void* page_phyaddr = palloc(mem_pool);
        if (page_phyaddr == NULL) {
            return NULL;
        }

        // 3. 物理页与虚拟页映射
        page_table_add((void*) vaddr, page_phyaddr);
        vaddr += PAGE_SIZE;
        --count;
    }

    return vaddr_start;
}

/**
 * 在内核内存池中申请page_count个页
 */
void* get_kernel_pages(uint32_t page_count) {
    // lock_acquire(&kernel_pool.lock);
    void* vaddr = malloc_page(PF_KERNEL, page_count);
    if (vaddr != NULL) {
        memset(vaddr, 0, page_count * PAGE_SIZE);
    }
    // lock_release(&kernel_pool.lock);
    return vaddr;
}

/**
 * 在用户空间中申请page_count页内存，并返回其虚拟地址
 */
void* get_user_pages(uint32_t page_count) {
    // 可能有多个线程/进程同时申请
    lock_acquire(&user_pool.lock);
    void* vaddr = malloc_page(PF_USER, page_count);
    memset(vaddr, 0, page_count * PAGE_SIZE);
    lock_release(&user_pool.lock);
    return vaddr;    
}

/**
 * 将地址vaddr与pf池中的物理地址关联，仅支持一页内存分配
 */
void* get_a_page(enum pool_flags pf, uint32_t vaddr) {
    struct pool* mem_pool = (pf & PF_KERNEL) ? &kernel_pool : &user_pool;
    lock_acquire(&mem_pool->lock);

    // 虚拟地址对应的位图置1
    struct task_struct* cur = running_thread();
    int32_t bit_idx = -1;

    if (cur->pgdir != NULL && pf == PF_USER) {
        // 用户进程申请内存，修改进程对应的虚拟地址位图
        bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);
    } else if (cur->pgdir == NULL && pf == PF_KERNEL) {
        // 内核线程申请内存，修改kernel_vaddr
        bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PAGE_SIZE;
        ASSERT(bit_idx > 0);
        bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
    } else {
        PANIC("get_a_page: not allow kernel alloc userspace or user alloc kernelspace by get_a_page\n");
    }

    // 从内核内存池或用户内存池申请一页物理内存并修改位图
    void* page_phyaddr = palloc(mem_pool);
    if (page_phyaddr == NULL) {
        return NULL;
    }

    // 页表将vaddr和page_phyaddr关联起来
    page_table_add((void*) vaddr, page_phyaddr);

    lock_release(&mem_pool->lock);
    return (void*) vaddr;
}

/**
 * 将给定的虚拟地址转为物理地址
 */
uint32_t addr_v2p(uint32_t vaddr) {
    uint32_t* pte = pte_ptr(vaddr);
    return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}

/**
 * 返回arena中第idx个内存块的地址
 */
static struct mem_block* arena2block(struct arena* arena, uint32_t idx) {
    return (struct mem_block*) ((uint32_t)arena + sizeof(struct arena) + idx * arena->desc->block_size);
}

/**
 * 返回内存块block所在的arena地址
 */
static struct arena* block2arena(struct mem_block* block) {
    return (struct arena*) ((uint32_t)block & 0xfffff000);
}

/**
 * 在堆中申请size字节内存
 */
void* sys_malloc(uint32_t size) {
    enum pool_flags pf;
    struct pool* mem_pool;
    uint32_t pool_size;
    struct mem_block_desc* descs;
    struct task_struct* cur_thread = running_thread();

    if (cur_thread->pgdir == NULL) {
        // 内核线程
        pf = PF_KERNEL;
        pool_size = kernel_pool.pool_size;
        mem_pool = &kernel_pool;
        descs = k_block_descs;
    } else {
        // 用户进程
        pf = PF_USER;
        pool_size = user_pool.pool_size;
        mem_pool = &user_pool;
        descs = cur_thread->u_block_desc;
    }

    // 申请的内存不在内存池容量范围内，返回NULL
    if (size <=0 || size >= pool_size) {
        return NULL;
    }

    struct arena* arena;
    struct mem_block* block;
    lock_acquire(&mem_pool->lock);

    if (size > 1024) {
        // 若申请的内存块超过1024字节，直接分配页框
        // 1. 向上取整得到要分配的页框数
        uint32_t page_cnt = DIV_ROUND_UP(size + sizeof(struct arena), PAGE_SIZE);
        arena = malloc_page(pf, page_cnt);
        if (arena == NULL) {
            lock_release(&mem_pool->lock);
            return NULL;
        }
        // 2. 分配内存块
        memset(arena, 0, page_cnt * PAGE_SIZE);
        arena->desc = NULL;
        arena->cnt = page_cnt;
        arena->large = TRUE;
        lock_release(&mem_pool->lock);
        return (void*) (arena + 1);

    } else {
        // 若申请的内存块小于等于1024，在mem_block_desc中分配
        uint8_t desc_idx;
        // 1. 先找到最合适大小的mem_block_desc
        for (desc_idx = 0; desc_idx < MEM_BLOCK_DESC_CNT; desc_idx++) {
            if (size <= descs[desc_idx].block_size) {
                break;
            }
        }
        // 2. 如果mem_block_desc.free_list没有可用的mem_block，则创建新的arena
        if (list_empty(&descs[desc_idx].free_list)) {
            // 申请1页内存创建新的arena
            arena = malloc_page(pf, 1);
            if (arena == NULL) {
                lock_release(&mem_pool->lock);
                return NULL;
            }
            memset(arena, 0, PAGE_SIZE);
            arena->desc = &descs[desc_idx];
            arena->cnt = descs[desc_idx].blocks_per_arena;
            arena->large = FALSE;
            // 将arena的内存块加入到mem_block_desc.free_list中
            uint32_t block_idx;
            enum intr_status old_status = intr_disable();
            for (block_idx = 0; block_idx < descs[desc_idx].blocks_per_arena; block_idx++) {
                block = arena2block(arena, block_idx);
                ASSERT(!list_find(&arena->desc->free_list, &block->free_elem));
                list_append(&arena->desc->free_list, &block->free_elem);
            }
            intr_set_status(old_status);
        }
        // 3. 分配内存块，从free_list取出1块可用内存作为block
        // 这里用elem2entry宏，将mem_block.free_elem的地址转为mem_block的地址
        block = elem2entry(struct mem_block, free_elem, list_pop(&(descs[desc_idx].free_list)));
        memset(block, 0, descs[desc_idx].block_size);
        arena = block2arena(block);
        arena->cnt--;
        lock_release(&mem_pool->lock);
        return (void*) block;
    }
}

/**
 * 初始化内存块描述符mem_block_desc。为malloc做准备
 */
void block_desc_init(struct mem_block_desc* desc_array) {
    uint16_t desc_idx;
    uint16_t block_size = 16;

    // 初始化内存块描述符，大小：16, 32, ... , 1024字节
    for (desc_idx = 0; desc_idx < MEM_BLOCK_DESC_CNT; desc_idx++) {
        desc_array[desc_idx].block_size = block_size;
        desc_array[desc_idx].blocks_per_arena = (PAGE_SIZE - sizeof(struct arena)) / block_size;
        list_init(&desc_array[desc_idx].free_list);
        block_size *= 2;
    }
}

/**
 * 内存管理入口
 */
void mem_init(void) {
    put_str("mem_init start\n");
    lock_init(&kernel_pool.lock);
    lock_init(&user_pool.lock);
    // 从loader.S读取total_mem_bytes，位置是0x900+0x100。Makefile实现是32MB
    uint32_t total_memory = (*(uint32_t*) (0xb00));
    mem_pool_init(total_memory);
    block_desc_init(k_block_descs);
    put_str("mem_init done\n");
}
