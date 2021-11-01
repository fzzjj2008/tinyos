#include "boot.h"

.code16
.section .text
.globl main
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
total_mem_bytes:
    .int 0
ards_buf:
    .fill 256, 1, 0
ards_nr:
    .word 0


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
# 获取内存，参考Linux实现，使用三种方式获取
##################################################
.set SMAP,     0x534d4150           # ascii 'SMAP'
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
# 保护模式
##################################################
.code32
p_mode_start:
    movl $SELECTOR_DATA, %eax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movl $LOADER_STACK_TOP, %esp
    movl $SELECTOR_VIDEO, %eax
    movw %ax, %gs

# 从磁盘加载kernel.bin
load_kernel:
    movl $KERNEL_START_SECTOR, %eax     # 起始扇区LBA地址
    movl $KERNEL_BIN_BASE_ADDR, %ebx    # 将数据写入的内存地址
    movl $200, %ecx                     # 读入的扇区数
    call rd_disk_m_32

# 创建页目录及页表，初始化页内存位图
page_table_init:
    call setup_page                 # 创建页目录及页表
    sgdt gdt_ptr                    # 先保存gdtr到内存中

    # 4G虚拟空间划分为：用户空间0~0xbfffffff、内核空间0xc0000000~0xffffffff
    # 由于打印功能放内核实现，故显存段段基址改为3G以上
    movl (gdt_ptr+2), %ebx
    addl $SELECTOR_VIDEO, %ebx
    orl $0xc0000000, 4(%ebx)        # GDT显存段段基址=0xc00b8000(此处段描述符31~24位是段基址31~24位，或运算修改段基址)
    addl $0xc0000000, (gdt_ptr+2)   # GDT基址=0xc0000900，将GDT移到内核空间中
    addl $0xc0000000, %esp          # 栈地址修改成内核使用的地址

    movl $PAGE_DIR_TABLE_POS, %eax  # 页目录地址赋给CR3
    movl %eax, %cr3

    movl %cr0, %eax                 # 页目录地址赋给CR3
    orl  $CR0_PG_ON, %eax
    movl %eax, %cr0

    lgdt gdt_ptr                    # 重新加载gdtr

    # 输出P
    movb $'P',  %gs:(0xa0)
    movb $0xA4, %gs:(0xa1)

# 进入kernel的main.c
enter_kernel:
    call kernel_init
    movl $0xc009f000, %esp
    jmp KERNEL_ENTRY_POINT
spin:
    jmp spin


##################################################
# 创建页目录及页表 (实际只初始化了page_dir和page_table1)
#   page_dir:     0x100000~0x100fff, 1024项, 4096 bytes
#     user_space: 0x101000~0x101bff, 768项
#     kern_space: 0x101c00~0x101fff, 256项
#   page_table1:  0x101000~0x101fff, 1024项, 4096 bytes, 对应物理地址0~0x3fffff (实际tinyos只用了0~0xfffff 1MB内存)
#   page_table2:  0x102000~0x102fff, 1024项, 4096 bytes, 对应物理地址0x400000~0x7fffff (后续物理地址未使用)
#   ...
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
    movl %eax, (PAGE_DIR_TABLE_POS)        # PDE0=0x101007, 指向页表1地址0x101000, 权限为US=1,RW=1,P=1
    movl %eax, (PAGE_DIR_TABLE_POS+0xc00)  # PDE768=0x101007, 指向页表1地址0x101000, 权限为US=1,RW=1,P=1
    subl $0x1000, %eax
    movl %eax, (PAGE_DIR_TABLE_POS+4092)   # PDE1023=0x100007, 指向页目录表本身0x100000, 权限为US=1,RW=1,P=1

# 创建页表项PTE
    movl $256, %ecx                 # 填充页表1的0~255项。1张页表可表示4MB物理空间，实际tinyos只用了1MB内存，剩下没必要填充
    movl $0, %esi
    movl $PTE_U+PTE_W+PTE_P, %edx
create_pte:
    movl %edx, (%ebx, %esi, 4)      # PTE0=0x000007, 指向物理地址0x0, 权限为US=1,RW=1,P=1
    addl $4096, %edx                # PTE1=0x001007, 指向物理地址0x1000, 权限为US=1,RW=1,P=1
    incl %esi                       # ...
    loop create_pte                 # PTE255=0x0FF007, 指向物理地址0xFF000, 权限为US=1,RW=1,P=1

# 创建内核其它页表的PDE。此处只有用到页表1 0x101000，其它页表实际没用到。
    movl $PAGE_DIR_TABLE_POS, %eax
    addl $0x2000, %eax
    orl $PTE_U+PTE_W+PTE_P, %eax
    movl $PAGE_DIR_TABLE_POS, %ebx  # eax=0x102007, ebx=0x100000
    movl $254, %ecx                 # 填充页目录表的769~1022项
    movl $769, %esi
create_kernel_pde:
    movl %eax, (%ebx, %esi, 4)      # PDE769=0x102007, 指向物理地址0x102000, 权限为US=1,RW=1,P=1
    incl %esi                       # PDE770=0x103007, 指向物理地址0x103000, 权限为US=1,RW=1,P=1
    addl $0x1000, %eax              # ...
    loop create_kernel_pde          # PDE1022=0x1FF007, 指向物理地址0x1ff000, 权限为US=1,RW=1,P=1
    ret


##################################################
# 保护模式读磁盘
##################################################
rd_disk_m_32:
    movl %eax, %esi                 # 备份eax
    movw %cx, %di                   # 备份cx

# 第1步：设置读取的扇区数
    movl $0x1f2, %edx
    movb %cl, %al
    outb %al, %dx

    movl %esi, %eax                 # 恢复eax

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
    or $0xe0, %al                   # 设置7~4位为1110, 表示LBA模式
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
    and $0x88, %al                  # status寄存器第3位表示硬件准备好数据，可以输出了
    cmp $0x08, %al                  # status寄存器第7位1表示硬盘正忙
    jnz not_ready

# 第5步：从0x1f0端口读数据
    movw %di, %ax
    movl $256, %edx                 # 一个扇区有512字节，总共读取256次
    mulw %dx
    movw %ax, %cx
    movl $0x1f0, %edx
go_on_read:
    inw %dx, %ax                    # 每次读取1个字，即2个字节
    movw %ax, (%ebx)
    addl $2, %ebx
    loop go_on_read
    ret


##################################################
# 初始化内核
##################################################
kernel_init:
    xorl %eax, %eax
    xorl %ebx, %ebx
    xorl %ecx, %ecx
    xorl %edx, %edx
    movw (KERNEL_BIN_BASE_ADDR+42), %dx     # e_phentsize, 段表项大小
    movl (KERNEL_BIN_BASE_ADDR+28), %ebx    # e_phoff, 段表偏移
    addl $KERNEL_BIN_BASE_ADDR, %ebx        # ebx=0x70034, 段表地址
    movw (KERNEL_BIN_BASE_ADDR+44), %cx     # e_phnum, 段表项数目

each_segment:
    cmpb $0, (%ebx)
    je PTNULL                               # p_type=PT_NULL=0，则指向下一个段表

    pushl 16(%ebx)                          # p_filesz，段大小，mem_cpy第三个参数size=60
    movl 4(%ebx), %eax                      # p_offset
    addl $KERNEL_BIN_BASE_ADDR, %eax        # 段物理地址
    pushl %eax                              # mem_cpy第二个参数src=0x70074
    pushl 8(%ebx)                           # mem_cpy第一个参数dst=0xc0001500
    call mem_cpy
    addl $12, %esp                          # 弹出栈
PTNULL:
    addl %edx, %ebx                         # ebx指向下一个段表项
    loop each_segment
    ret

# 逐字节拷贝mem_cpy(dst, src, size)
mem_cpy:
    cld
    pushl %ebp
    movl %esp, %ebp
    pushl %ecx
    movl 8(%ebp), %edi              # dst
    movl 12(%ebp), %esi             # src
    movl 16(%ebp), %ecx             # size
    rep movsb                       # 逐字节拷贝
    popl %ecx
    popl %ebp
    ret
