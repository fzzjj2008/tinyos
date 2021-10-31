#include "boot.h"

.code16
.section .text
.set SELECTOR_CODE,     0x8         # code segment selector, index=0x1, ti=0, rpl=00
.set SELECTOR_DATA,     0x10        # data segment selector, index=0x2, ti=0, rpl=00
.set SELECTOR_VIDEO,    0x18        # video segment selector, index=0x3, ti=0, rpl=00
.set CR0_PE_ON,         0x1         # protected mode enable flag
.set CR0_PG_ON,         0x80000000  # paging enable flag

##################################################
# GDT表 0x900~0x920
#   null seg: 0
#   code seg: base=0x0, lim=0xfffff
#   data seg: base=0x0, lim=0xfffff
#   video seg: base=0xb8000, lim=(0xbffff-0xb8000)/4k=0x7
##################################################
.p2align 2                          # force 4 byte alignment
gdt:
    SEG_ASM_NULL                    # null segment
    SEG_ASM(STA_X, 0x0, 0xfffff)    # code segment
    SEG_ASM(STA_W, 0x0, 0xfffff)    # data segment
    SEG_ASM(STA_W, 0xb8000, 0x7)    # video segment
# GDTR寄存器
gdt_ptr:
    .word 4*8-1                     # GDT界限, 支持4个GDT
    .long gdt                       # GDT内存起始地址


##################################################
# 获取内存，参考Linux实现，使用三种方式获取
##################################################
.set SMAP,     0x534d4150           # ascii 'SMAP'
total_mem_bytes:
    .int 0
ards_buf:
    .fill 256, 1, 0
ards_nr:
    .word 0

get_mem_size:
# 使用BIOS 0x15中断0xE820子功能获取内存
mem_e820:
    xorl %ebx, %ebx
    movw %bx, %es
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
    addl $20, %ebx                  # ebx指向下一个ARDS结构体
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
    # 输出OK, edx = 0x2000000 = 32 MB
    # i r edx
    # x/x total_mem_bytes
    movl %edx, (total_mem_bytes)
    movl $0xb800, %eax
    movw %ax, %es
    movb $'O',  %es:(0x0)
    movb $'K',  %es:(0x2)
    ret
mem_get_fail:
    # 输出FAIL
    movl $0xb800, %eax
    movw %ax, %es
    movb $'F',  %es:(0x0)
    movb $'A',  %es:(0x2)
    movb $'I',  %es:(0x4)
    movb $'L',  %es:(0x6)
    ret


##################################################
# 创建页目录及页表
#   page_dir:     0x100 000~0x100 fff, 1024项, 4096 bytes
#     user_space: 0x101 000~0x101 bff
#     kern_space: 0x101 c00~0x101 fff
#   page_table1:  0x101 000~0x101 fff, 1024项, 4096 bytes, 对应0~0x3fffff之间的物理页
##################################################
setup_page:
# 清理页目录占用空间
    movl $4096, %ecx
    movl $PAGE_DIR_TABLE_POS, %esi
clear_page_dir:
    movb $0, (%esi)
    incl %esi
    loop clear_page_dir

# 创建页目录表项PDE
create_pde:
    movl $PAGE_DIR_TABLE_POS, %eax
    addl $0x1000, %eax
    movl %eax, %ebx                 # eax=ebx=0x101000
    orl $PTE_U+PTE_W+PTE_P, %eax
    movl %eax, (%ebx)               # PDE0=0x101007, 指向页表1地址0x101000, 权限为US=1,RW=1,P=1
    movl %eax, 0xc00(%ebx)          # PDE768=0x101007, 指向页表1地址0x101000, 权限为US=1,RW=1,P=1
    subl $0x1000, %eax
    movl %eax, 4092(%ebx)           # PDE1023=0x100007, 指向页目录表本身0x100000, 权限为US=1,RW=1,P=1

# 创建页表项PTE
    movl $256, %ecx                 # 填充页表1的0~255项。1张页表可表示4MB物理空间，实际tinyos只用了1MB内存，剩下没必要填充
    movl $0, %esi
    movl $PTE_U+PTE_W+PTE_P, %edx
create_pte:
    movl %edx, (%ebx, %esi, 4)      # PTE0=0x101007, 指向物理地址0x0, 权限为US=1,RW=1,P=1
    addl $4096, %edx                # PTE1=0x102007, 指向物理地址0x1000, 权限为US=1,RW=1,P=1
    incl %esi                       # ...
    loop create_pte                 # PTE255=0x200007, 指向物理地址0xFF000, 权限为US=1,RW=1,P=1

# 创建内核其它页表的PDE
    movl $PAGE_DIR_TABLE_POS, %eax
    addl $0x2000, %eax
    orl $PTE_U+PTE_W+PTE_P, %eax
    movl $PAGE_DIR_TABLE_POS, %ebx  # eax=0x102007, ebx=0x100000
    movl $254, %ecx                 # 填充页目录表的769~1022项
    movl $769, %esi
create_kernel_pde:
    movl %eax, (%ebx, %esi, 4)      # PDE769=0x101007, 指向物理地址0x0, 权限为US=1,RW=1,P=1
    incl %esi                       # PDE770=0x101007, 指向物理地址0x0, 权限为US=1,RW=1,P=1
    addl $0x1000, %eax              # ...
    loop create_kernel_pde          # PDE1022=0x101007, 指向物理地址0x0, 权限为US=1,RW=1,P=1
    ret


##################################################
# 实模式
##################################################
.org 0x300
main:
    # 获取内存
    call get_mem_size

# 进入保护模式
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

# 创建页目录及页表，初始化页内存位图
page_table_init:
    # 创建页目录及页表
    call setup_page
    # 先保存gdtr到内存中
    sgdt gdt_ptr
    
    movl (gdt_ptr+2), %ebx
    addl $SELECTOR_VIDEO, %ebx 
    orl $0xc0000000, 4(%ebx)        # GDT VIDEO SEG段基址加上0xc0000000(段描述符31~24位是段基址31~24位)
    addl $0xc0000000, (gdt_ptr+2)   # GDT基址加上0xc0000000称为内核所在高地址
    addl $0xc0000000, %esp          # 栈指针映射到内核地址

    movl $PAGE_DIR_TABLE_POS, %eax  # 页目录地址赋给CR3
    movl %eax, %cr3

    movl %cr0, %eax
    orl  $CR0_PG_ON, %eax
    movl %eax, %cr0

    # 重新加载gdtr
    lgdt gdt_ptr

    # 输出P
    movb $'P',  %gs:(0xa0)
    movb $0xA4, %gs:(0xa1)
spin:
    jmp spin
