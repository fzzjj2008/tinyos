#include "boot.h"

.code16
.section .text
.set SELECTOR_CODE,     0x8         # code segment selector, index=0x1, ti=0, rpl=00
.set SELECTOR_DATA,     0x10        # data segment selector, index=0x2, ti=0, rpl=00
.set SELECTOR_VIDEO,    0x18        # video segment selector, index=0x3, ti=0, rpl=00
.set CR0_PE_ON,         0x1         # protected mode enable flag

##################################################
# 实模式
##################################################
.globl main
main:
    # 获取内存
    call get_mem_size

enter_protect_mode:
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
.set SMAP,     0x534d4150           # ascii 'SMAP'
total_mem_bytes:
    .int 0
ards_buf:
    .rept 256
    .byte 0
    .endr
ards_nr:
    .word 0

get_mem_size:
# 使用BIOS 0x15中断0xE820子功能获取内存
mem_e820:
    xorl %ebx, %ebx
    movl $SMAP, %edx
    movl $ards_buf, %edi
e820_loop:
    movl $0xe820, %eax
    movl $20, %ecx                  # ARDS地址范围描述符结构大小是20字节
    int $0x15
    jc mem_e801                     # 获取失败尝试0xE801子功能获取

    addw %cx, %di                   # ARDS地址+20，指向新的ARDS结构体
    incw (ards_nr)
    cmpl $0, %ebx                   # 若ebx为0且cf不为1，说明ARDS全部返回
    jnz e820_loop

# 遍历ARDS结构体，找到(base_add_low + length_low)的最大值，即内存的容量
# 实际内存大小edx = max{base_add_low + length_low}
    movw (ards_nr), %cx
    movl $ards_buf, %ebx            # ebx指向第一个ARDS结构体
    xorl %edx, %edx                 # edx = 0
e820_find_max_mem_area:
    movl (%ebx), %eax               # base_add_low
    addl 8(%ebx), %eax              # eax = base_add_low + length_low
    addl 20, %ebx                   # ebx指向下一个ARDS结构体
    cmpl %eax, %edx                 # if (eax > edx) { edx = eax; }
    jge e820_next_ards
    movl %eax, %edx
e820_next_ards:
    loop e820_find_max_mem_area
    jmp mem_get_ok

# 使用BIOS 0x15中断0xE801子功能获取内存
# 实际内存大小edx = 低15MB内存 + 16MB~4GB内存
mem_e801:
    movw $0xe801, %ax
    int $0x15
    jc mem_88                       # 获取失败尝试0x88子功能获取

    # 计算低15MB内存
    movw $0x400, %cx
    mulw %cx                        # edx = cx * 1024 (byte)
    shll $16, %edx
    andl $0x0000FFFF, %eax
    orl %eax, %edx                  # 原edx保留乘积的高16位，eax保留乘积的低16位。edx左移拼接成32位内存结果edx
    addl $0x100000, %edx            # 获取的大小比实际少1MB，补偿1MB
    movl %edx, %esi

    # 计算16MB~4GB内存
    xorl %eax, %eax                 # eax = 0
    movw %bx, %ax
    movl $0x10000, %ecx             # edx = bx * 64 * 1024 (byte)
    mull %ecx                       # 原edx保留乘积的高32位(内存<4GB，故肯定是全0)，eax保留乘积的低32位

    # 计算总大小
    addl %eax, %esi
    movl %esi, %edx
    jmp mem_get_ok

# 使用BIOS 0x15中断0x88子功能获取内存
mem_88:
    movb $0x88, %ah
    int $0x15
    jc mem_get_fail                 # 获取内存失败
    andl $0x0000FFFF, %eax

    movw $0x400, %cx
    mulw %cx                        # edx = bx * 1024 (byte)
    shll $16, %edx
    orl %eax, %edx                  # 原edx保留乘积的高16位，eax保留乘积的低16位。edx左移拼接成32位内存结果edx
    addl $0x100000, %edx            # 获取的大小比实际少1MB，补偿1MB

mem_get_ok:
    movl %edx, (total_mem_bytes)
    ret
mem_get_fail:
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
