#include "boot.h"

.code16
.section .text
.globl main
main:
    # 显示2 LOADER
    movw $0xb800, %ax
    movw %ax, %es
    xorw %di, %di
    movb $'2',  %es:(%di)
    movb $0xA4, %es:0x1(%di)
    movb $' ',  %es:0x2(%di)
    movb $0xA4, %es:0x3(%di)
    movb $'L',  %es:0x4(%di)
    movb $0xA4, %es:0x5(%di)
    movb $'O',  %es:0x6(%di)
    movb $0xA4, %es:0x7(%di)
    movb $'A',  %es:0x8(%di)
    movb $0xA4, %es:0x9(%di)
    movb $'D',  %es:0xa(%di)
    movb $0xA4, %es:0xb(%di)
    movb $'E',  %es:0xc(%di)
    movb $0xA4, %es:0xd(%di)
    movb $'R',  %es:0xe(%di)
    movb $0xA4, %es:0xf(%di)

spin:
    jmp spin
