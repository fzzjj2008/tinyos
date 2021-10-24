.code16
.section .text
.globl main
main:
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    # 屏幕初始化，清屏
    movw $0x600, %ax
    movw $0x700, %bx
    movw $0, %cx
    movw $0x184f, %dx
    int $0x10

    # 设置光标位置，BH = 页号，DH = 行，DL = 列
    movb $2, %ah
    movb $0, %bh
    movw $0x0a20, %dx
    int $0x10

    call display2
spin:
    jmp spin

display:
    movw $message, %ax
    movw %ax, %bp
    movw $14, %cx
    movw $0x1301, %ax
    movw $0x2, %bx
    int $0x10
    ret

display1:
    # 输出绿底红色跳动字符串
    # 0xb800: 'H' 0xA4 'e' 0xA4 'l' 0xA4 ...
    movw $0xb800, %ax
    movw %ax, %es
    xorw %di, %di
    movb $'H', %es:(%di)
    movb $0xA4, %es:1(%di)
    movb $'i', %es:2(%di)
    movb $0xA4, %es:3(%di)
    ret

display2:
    movw $0xb800, %ax
    movw %ax, %es
    movw $message, %si
    xorw %di, %di
display_single:
    movb $0, %ch
    movb (%si), %cl
    jcxz display_ok
    movb %cl, %es:(%di)
    movb $0xA4, %es:1(%di)
    inc %si
    addw $2, %di
    jmp display_single
display_ok:
    ret

message:
    .asciz "Hello, Tiny OS"

.org 510
    .word 0xAA55
