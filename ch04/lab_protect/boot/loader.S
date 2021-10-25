#include "boot.h"

.set SELECTOR_CODE,     0x8       # code segment selector, index=0x1, ti=0, rpl=00
.set SELECTOR_DATA,     0x10      # data segment selector, index=0x2, ti=0, rpl=00
.set SELECTOR_VIDEO,    0x18      # video segment selector, index=0x3, ti=0, rpl=00
.set CR0_PE_ON,         0x1       # protected mode enable flag

.text
.globl main
main:
.code16
    # 打印字符串loadermsg
    movw $LOADER_BASE_ADDR, %sp
    movw $loadermsg, %bp
    movw $17, %cx
    movw $0x1301, %ax
    movw $0x001f, %bx
    movw $0x1800, %dx
    int $0x10

    # 打开A20
    inb $0x92, %al
    or $2, %al
    outb %al, $0x92

    # 加载GDT
    lgdt (gdt_ptr)

    # CR0 PE位开启保护模式
    movl %cr0, %eax
    orl  $CR0_PE_ON, %eax
    movl %eax, %cr0

    ljmp $SELECTOR_CODE, $p_mode_start

loadermsg:
    .ascii "2 loader in real."

# 构建GDT表
#   null seg: 0
#   code seg: base=0x0, lim=0xfffff
#   data seg: base=0x0, lim=0xfffff
#   video seg: base=0xb8000/4k=0xb8, lim=(0xbffff-0xb8000)/4k=0x7
gdt:
    SEG_ASM_NULL                                   # null segment
    SEG_ASM(STA_X, 0x0, 0xfffff)                   # code segment
    SEG_ASM(STA_W, 0x0, 0xfffff)                   # data segment
    SEG_ASM(STA_W, 0xb8, 0x7)                      # video segment
gdt_ptr:
    .word GDT_LIMIT
    .long gdt


.code32
p_mode_start:
    movl $SELECTOR_DATA, %eax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %ss
    movl $LOADER_STACK_TOP, %esp
    movl $SELECTOR_VIDEO, %eax
    movw %ax, %gs

    # 输出P
    xorl %edi, %edi
    movb $'P',  %gs:160(%di)
    movb $0xA4, %gs:161(%di)
spin:
    jmp spin
