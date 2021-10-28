#ifndef __BOOT_H__
#define __BOOT_H__

/* loader */
#define LOADER_BASE_ADDR      0x900
#define LOADER_START_SECTOR   0x2

/* gdt */
// 第0个段描述符，8字节
#define SEG_ASM_NULL                                            \
    .word 0, 0, 0, 0

// 数据段段描述符构造
// TYPE=段属性, BASE=段基址, LIM=段界限, G=1, D/B=1, L=0, AVL=0, P=1, DPL=00, S=1 
#define SEG_ASM(type,base,lim)                                  \
    .word ((lim) & 0xffff), ((base) & 0xffff);                  \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),             \
        (0xC0 | (((lim) >> 16) & 0xf)), (((base) >> 24) & 0xff)

// 段描述符段属性
#define STA_X       0x8     // 可执行
#define STA_E       0x4     // 扩展方向
#define STA_C       0x4     // 一致性代码段
#define STA_W       0x2     // 可写
#define STA_R       0x2     // 可读
#define STA_A       0x1     // 可访问

#endif /*__BOOT_H__*/
