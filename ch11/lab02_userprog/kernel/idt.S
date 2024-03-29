#include "idt.h"

.code32
.section .data
.globl intr_entry_table
.globl intr_exit
intr_str:
    .asciz "interrupt occur!\n"

.macro INTR_NO_ERRCODE nr
.section .text
intr_entry_\nr:
    pushl $0                    # 无ERRORCODE中断，push $0，保证最后exit流程一致
    #push $intr_str
    #call put_str
    #add $4, %esp

    push %ds
    push %es
    push %fs
    push %gs
    pushal

    movb $0x20, %al             # 中断结束命令EOI
    outb %al, $0xa0             # 向从片发送
    outb %al, $0x20             # 向主片发送

    pushl $\nr
    call *(idt_table+\nr*4)     # 调用idt_table[nr]的函数指针
    jmp intr_exit
.section .data
    .long intr_entry_\nr
.endm

.macro INTR_ERRCODE nr
.section .text
intr_entry_\nr:                 # 有ERRORCODE中断，默认栈会压入ERRORCODE
    #push $intr_str
    #call put_str
    #add $4, %esp

    push %ds
    push %es
    push %fs
    push %gs
    pushal

    movb $0x20, %al             # 中断结束命令EOI
    outb %al, $0xa0             # 向从片发送
    outb %al, $0x20             # 向主片发送

    pushl $\nr
    call *(idt_table+\nr*4)     # 调用idt_table[nr]的函数指针
    jmp intr_exit
.section .data
    .long intr_entry_\nr
.endm

intr_exit:
    addl $4, %esp
    popal
    pop %gs
    pop %fs
    pop %es
    pop %ds
    addl $4, %esp
    iret

intr_entry_table:
    INTR_NO_ERRCODE 0x00        # 0~19为处理器内部固定异常, 20~31为Intel保留（表7-1）
    INTR_NO_ERRCODE 0x01
    INTR_NO_ERRCODE 0x02
    INTR_NO_ERRCODE 0x03
    INTR_NO_ERRCODE 0x04
    INTR_NO_ERRCODE 0x05
    INTR_NO_ERRCODE 0x06
    INTR_NO_ERRCODE 0x07
    INTR_NO_ERRCODE 0x08
    INTR_NO_ERRCODE 0x09
    INTR_NO_ERRCODE 0x0a
    INTR_NO_ERRCODE 0x0b
    INTR_NO_ERRCODE 0x0c
    INTR_NO_ERRCODE 0x0d
    INTR_NO_ERRCODE 0x0e
    INTR_NO_ERRCODE 0x0f
    INTR_NO_ERRCODE 0x10
    INTR_NO_ERRCODE 0x11
    INTR_NO_ERRCODE 0x12
    INTR_NO_ERRCODE 0x13
    INTR_NO_ERRCODE 0x14
    INTR_NO_ERRCODE 0x15
    INTR_NO_ERRCODE 0x16
    INTR_NO_ERRCODE 0x17
    INTR_NO_ERRCODE 0x18
    INTR_NO_ERRCODE 0x19
    INTR_NO_ERRCODE 0x1a
    INTR_NO_ERRCODE 0x1b
    INTR_NO_ERRCODE 0x1c
    INTR_NO_ERRCODE 0x1d
    INTR_NO_ERRCODE 0x1e        # 书中这里设为ERRORCODE，实测是NO_ERRORCODE
    INTR_NO_ERRCODE 0x1f
    INTR_NO_ERRCODE 0x20        # 时钟中断对应入口
    INTR_NO_ERRCODE 0x21        # 键盘中断对应入口
    INTR_NO_ERRCODE 0x22        # 级联使用
    INTR_NO_ERRCODE 0x23        # 串口2对应入口
    INTR_NO_ERRCODE 0x24        # 串口1对应入口
    INTR_NO_ERRCODE 0x25        # 并口2对应入口
    INTR_NO_ERRCODE 0x26        # 软盘对应入口
    INTR_NO_ERRCODE 0x27        # 并口1对应入口
    INTR_NO_ERRCODE 0x28        # 实时时钟对应入口
    INTR_NO_ERRCODE 0x29        # 重定向
    INTR_NO_ERRCODE 0x2a        # 保留
    INTR_NO_ERRCODE 0x2b        # 保留
    INTR_NO_ERRCODE 0x2c        # ps/2鼠标
    INTR_NO_ERRCODE 0x2d        # fpu浮点单元异常
    INTR_NO_ERRCODE 0x2e        # 硬盘
    INTR_NO_ERRCODE 0x2f        # 保留
