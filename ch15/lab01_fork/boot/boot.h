#ifndef __BOOT_H__
#define __BOOT_H__

/**************** loader和kernel ****************/
#define LOADER_START_SECTOR     0x2             // Boot Loader起始扇区
#define LOADER_BASE_ADDR        0x900           // Boot Loader内存起始地址
#define LOADER_STACK_TOP        0x900           // 保护模式栈顶地址
#define PAGE_DIR_TABLE_POS      0x100000        // 页目录表起始地址
#define KERNEL_START_SECTOR     0x9             // kernel起始扇区
#define KERNEL_BIN_BASE_ADDR    0x70000         // kernel.bin加载磁盘地址
#define KERNEL_ENTRY_POINT      0xc0001500      // Kernel内存起始地址


/********************* gdt **********************/
// 第0个段描述符，8字节
#define SEG_ASM_NULL                                            \
    .word 0, 0, 0, 0

// 数据段段描述符，8字节
// TYPE=段属性, BASE=段基址, LIM=段界限, G=1, D/B=1, L=0, AVL=0, P=1, DPL=00, S=1 
#define SEG_ASM(type,base,lim)                                  \
    .word ((lim) & 0xffff), ((base) & 0xffff);                  \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),             \
        (0xC0 | (((lim) >> 16) & 0xf)), (((base) >> 24) & 0xff)

// 段描述符段属性
#define STA_X                   0x8             // 可执行
#define STA_E                   0x4             // 扩展方向
#define STA_C                   0x4             // 一致性代码段
#define STA_W                   0x2             // 可写
#define STA_R                   0x2             // 可读
#define STA_A                   0x1             // 可访问


/******************** 页表 **********************/
#define PTE_P                   0x001           // Present
#define PTE_W                   0x002           // Writeable
#define PTE_U                   0x004           // User
#define PTE_PWT                 0x008           // Write-Through
#define PTE_PCD                 0x010           // Cache-Disable
#define PTE_A                   0x020           // Accessed
#define PTE_D                   0x040           // Dirty
#define PTE_PS                  0x080           // Page Size
#define PTE_MBZ                 0x180           // Bits must be zero
#define PTE_AVAIL               0xE00           // Available for software use


#endif /*__BOOT_H__*/
