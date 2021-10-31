#include "boot.h"

.code16
.section .text
.set SELECTOR_CODE,     0x8       # code segment selector, index=0x1, ti=0, rpl=00
.set SELECTOR_DATA,     0x10      # data segment selector, index=0x2, ti=0, rpl=00
.set SELECTOR_VIDEO,    0x18      # video segment selector, index=0x3, ti=0, rpl=00
.set CR0_PE_ON,         0x1       # protected mode enable flag

##################################################
# 实模式
##################################################
.globl main
main:
    # 打印字符串loadermsg
    movl $0xb800, %eax
    movw %ax, %es
    movb $'2',  %es:(0x10)
    movb $0xA4, %es:(0x11)
    movb $' ',  %es:(0x12)
    movb $0xA4, %es:(0x13)
    movb $'L',  %es:(0x14)
    movb $0xA4, %es:(0x15)
    movb $'O',  %es:(0x16)
    movb $0xA4, %es:(0x17)
    movb $'A',  %es:(0x18)
    movb $0xA4, %es:(0x19)
    movb $'D',  %es:(0x1a)
    movb $0xA4, %es:(0x1b)
    movb $'E',  %es:(0x1c)
    movb $0xA4, %es:(0x1d)
    movb $'R',  %es:(0x1e)
    movb $0xA4, %es:(0x1f)

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
# 保护模式
##################################################
.code32
p_mode_start:
    movl $SELECTOR_DATA, %eax
    movw %ax, %ds
    movw %ax, %es
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
