#include "boot.h"

.code16
.section .text
.set SELECTOR_CODE,     0x8           # code segment selector, index=0x1, ti=0, rpl=00
.set SELECTOR_DATA,     0x10          # data segment selector, index=0x2, ti=0, rpl=00
.set SELECTOR_VIDEO,    0x18          # video segment selector, index=0x3, ti=0, rpl=00
.set CR0_PE_ON,         0x1           # protected mode enable flag 

##################################################
# 实模式
##################################################
.globl main
main:
    # 获取内存
    call get_mem_size

    # 打开A20
    inb $0x92, %al
    or $2, %al
    outb %al, $0x92

    # 关闭中断
    cli
    # 加载GDT
    lgdt gdt_ptr

    # CR0 PE位开启保护模式
    movl %cr0, %eax
    orl  $CR0_PE_ON, %eax
    movl %eax, %cr0

    # 跳转到GDT代码段，并清空流水线
    ljmp $SELECTOR_CODE, $p_mode_start


##################################################
# 获取内存，参考Linux实现，使用三种方式获取
##################################################
#define     SMAP        0x534d4150    # ascii 'SMAP'
get_mem_size:
# 使用BIOS 0x15中断0xE820子功能获取内存
mem_e820:
    xorl %ebx, %ebx
    movl $SMAP, %edx
    movw $ards_buf, %di
loop_e820:
    movl $0xe820, %eax
    movl $20, %ecx
    int $0x15
    jc mem_e801

    addw %cx, %di
    incw (ards_nr)
    cmpl $0, %ebx
    jnz loop_e820

    movw (ards_nr), %cx
    movl $ards_buf, %ebx
    xorl %edx, %edx
e820_find_max_mem_area:
    movl (%ebx), %eax
    addl 8(%ebx), %eax
    addl 20, %ebx
    cmpl %eax, %edx
    jge e820_next_ards
    movl %eax, %edx
e820_next_ards:
    loop e820_find_max_mem_area
    jmp mem_get_ok

# 使用BIOS 0x15中断0x88子功能获取内存
mem_e801:
    movw $0xe801, %ax
    int $0x15
    jc mem_88

    # 计算低15MB内存
    movw $0x400, %cx
    mulw %cx
    shll $16, %edx
    andl $0x0000FFFF, %eax
    orl %eax, %edx
    addl $0x100000, %edx
    movl %edx, %esi
    # 计算16MB~4GB内存
    xorl %eax, %eax
    movw %bx, %ax
    movl $0x10000, %ecx
    mull %ecx
    # 计算总大小
    addl %eax, %esi
    movl %esi, %edx
    jmp mem_get_ok

# 使用BIOS 0x15中断0x88子功能获取内存
mem_88:
    movb $0x88, %ah
    int $0x15
    jc error_hlt
    andl $0x0000FFFF, %eax

    mov $0x400, %cx
    mul %cx
    shll $16, %edx
    or %eax, %edx
    add $0x100000, %edx

mem_get_ok:
    movl %edx, (total_mem_bytes)
    ret


##################################################
# 保护模式
##################################################
.code32
p_mode_start:
    movl $SELECTOR_DATA, %eax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %ss
    movl $LOADER_BASE_ADDR, %esp
    movl $SELECTOR_VIDEO, %eax
    movw %ax, %gs

    # 输出P
    movb $'P',  %gs:(0xa0)
    movb $0xA4, %gs:(0xa1)
spin:
    jmp spin


##################################################
# GDT表
#   null seg: 0
#   code seg: base=0x0, lim=0xfffff
#   data seg: base=0x0, lim=0xfffff
#   video seg: base=0xb8000, lim=(0xbffff-0xb8000)/4k=0x7
##################################################
.p2align 2                                         # force 4 byte alignment
gdt:
    SEG_ASM_NULL                                   # null segment
    SEG_ASM(STA_X, 0x0, 0xfffff)                   # code segment
    SEG_ASM(STA_W, 0x0, 0xfffff)                   # data segment
    SEG_ASM(STA_W, 0xb8000, 0x7)                   # video segment
# GDTR寄存器
gdt_ptr:
    .word 0x800          # GDT界限, 支持256个GDT
    .long gdt            # GDT内存起始地址
