#include "boot.h"

.code16
.section .text
.globl main
main:
    xorl %eax, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw $0x7c00, %sp
    movw $0xb800, %ax
    movw %ax, %es

    # 屏幕初始化，清屏
    movw $0x600, %ax
    movw $0x700, %bx
    movw $0, %cx
    movw $0x184f, %dx
    int $0x10

    # 显示1 MBR
    movb $'1',  %es:(0x0)
    movb $0xA4, %es:(0x1)
    movb $' ',  %es:(0x2)
    movb $0xA4, %es:(0x3)
    movb $'M',  %es:(0x4)
    movb $0xA4, %es:(0x5)
    movb $'B',  %es:(0x6)
    movb $0xA4, %es:(0x7)
    movb $'R',  %es:(0x8)
    movb $0xA4, %es:(0x9)

    # 读取第2块扇区，跳转执行BOOT LOADER
    movl $LOADER_START_SECTOR, %eax  # 起始扇区LBA地址
    movw $LOADER_BASE_ADDR, %bx      # 将数据写入的内存地址
    movw $4, %cx                     # 读入的扇区数
    call rd_disk_m_16
    jmp LOADER_BASE_ADDR


# 读磁盘
rd_disk_m_16:
    movl %eax, %esi        # 备份eax
    movw %cx, %di          # 备份cx

# 第1步：设置读取的扇区数
    movl $0x1f2, %edx
    movb %cl, %al
    outb %al, %dx

    movl %esi, %eax        # 恢复eax

# 第2步：将LBA地址存入0x1f3~0x1f6
    # LBA地址7~0位写入0x1f3
    movl $0x1f3, %edx
    outb %al, %dx

    # LBA地址15~8位写入0x1f4
    movb $8, %cl
    shr %cl, %eax
    movl $0x1f4, %edx
    outb %al, %dx

    # LBA地址23~16位写入0x1f5
    shr %cl, %eax
    movl $0x1f5, %edx
    outb %al, %dx

    # LBA地址24~27位写入0x1f6
    shr %cl, %eax
    and $0x0f, %al
    or $0xe0, %al        # 设置7~4位为1110, 表示LBA模式
    movl $0x1f6, %edx
    outb %al, %dx

# 第3步：向0x1f7端口写入读命令0x20
    movl $0x1f7, %edx
    movb $0x20, %al
    outb %al, %dx

# 第4步：检测硬盘状态
not_ready:
    nop
    inb %dx, %al
    and $0x88, %al    # status寄存器第3位表示硬件准备好数据，可以输出了
    cmp $0x08, %al    # status寄存器第7位1表示硬盘正忙
    jnz not_ready

# 第5步：从0x1f0端口读数据
    movw %di, %ax
    movl $256, %edx   # 一个扇区有512字节，总共读取256次
    mulw %dx
    movw %ax, %cx
    movl $0x1f0, %edx
go_on_read:
    inw %dx, %ax      # 每次读取1个字，即2个字节
    movw %ax, (%bx)
    addw $2, %bx
    loop go_on_read
    ret

.org 510
boot_flag:
    .word 0xAA55
