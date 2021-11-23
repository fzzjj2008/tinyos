#include "idt.h"

.code32
.section .data
intr_str:
    .asciz "interrupt occur!\n"

.macro INTR_NO_ERRCODE nr
.section .text
intr_entry_\nr:
    pushl $0
    #push $intr_str
    #call put_str
    #add $4, %esp

    push %ds
    push %es
    push %fs
    push %gs
    pushal

    movb $0x20, %al
    outb %al, $0xa0
    outb %al, $0x20

    pushl $\nr
    call *(idt_table+\nr*4)
    jmp intr_exit
.section .data
    .long intr_entry_\nr
.endm

.macro INTR_ERRCODE nr
.section .text
intr_entry_\nr:
    #push $intr_str
    #call put_str
    #add $4, %esp

    push %ds
    push %es
    push %fs
    push %gs
    pushal

    movb $0x20, %al
    outb %al, $0xa0
    outb %al, $0x20

    pushl $\nr
    call *(idt_table+\nr*4)
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

.globl intr_entry_table
intr_entry_table:
    INTR_NO_ERRCODE 0x00
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
    INTR_NO_ERRCODE 0x1e
    INTR_NO_ERRCODE 0x1f
    INTR_NO_ERRCODE 0x20
