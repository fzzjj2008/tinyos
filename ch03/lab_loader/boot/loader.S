#include "boot.h"

.code16
.section .text
.globl main
main:
    # 显示2 LOADER
    movw $0xb800, %ax
    movw %ax, %es
    xorw %di, %di
    movb $'2',  %es:0x10(%di)
    movb $0xA4, %es:0x11(%di)
    movb $' ',  %es:0x12(%di)
    movb $0xA4, %es:0x13(%di)
    movb $'L',  %es:0x14(%di)
    movb $0xA4, %es:0x15(%di)
    movb $'O',  %es:0x16(%di)
    movb $0xA4, %es:0x17(%di)
    movb $'A',  %es:0x18(%di)
    movb $0xA4, %es:0x19(%di)
    movb $'D',  %es:0x1a(%di)
    movb $0xA4, %es:0x1b(%di)
    movb $'E',  %es:0x1c(%di)
    movb $0xA4, %es:0x1d(%di)
    movb $'R',  %es:0x1e(%di)
    movb $0xA4, %es:0x1f(%di)

spin:
    jmp spin
