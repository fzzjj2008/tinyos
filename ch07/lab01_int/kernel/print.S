.code32
.set SELECTOR_VIDEO,    0x18        # video segment selector, index=0x3, ti=0, rpl=00
.section .data
put_int_buffer:
    .int 0, 0

##################################################
# 获取光标位置，光标范围25*80=0~1999
#   ax: 光标位置
##################################################
.section .text
.globl get_cursor
get_cursor:
    # pushl %ebp
    # movl %esp, %ebp
    pushl %edx

    # 获取当前光标位置高8位
    movw $0x03d4, %dx
    movb $0x0e, %al
    outb %al, %dx
    movw $0x03d5, %dx
    inb %dx, %al
    movb %al, %ah

    # 获取当前光标位置低8位
    movw $0x03d4, %dx
    movb $0x0f, %al
    outb %al, %dx
    movw $0x03d5, %dx
    inb %dx, %al

    popl %edx
    # leave                           # movl %ebp, %esp; popl %ebp
    ret


##################################################
# 设置光标位置，光标范围25*80=0~1999
#   bx: 光标位置
##################################################
.globl set_cursor
set_cursor:
    pushl %eax
    pushl %ebx
    pushl %edx
    movl 16(%esp), %ebx             # 返回值4字节，通用寄存器3*4=12字节

    # 设置高8位
    movw $0x03d4, %dx
    movb $0x0e, %al
    outb %al, %dx
    movw $0x03d5, %dx
    movb %bh, %al
    outb %al, %dx

    # 设置低8位
    movw $0x03d4, %dx
    movb $0x0f, %al
    outb %al, %dx
    movw $0x03d5, %dx
    movb %bl, %al
    outb %al, %dx

    popl %edx
    popl %ebx
    popl %eax
    ret


##################################################
# 打印字符
#   bx: 光标位置
#   cl: 栈中取出的打印字符
##################################################
.globl put_char
put_char:
    # 备份全部32位寄存器，入栈顺序：EAX->ECX->EDX->EBX->ESP->EBP->ESI->EDI
    pushal
    movl $SELECTOR_VIDEO, %eax       # 注意这里重新设gs，RPL=0，可以在内核态打印
    movw %ax, %gs

    call get_cursor
    movw %ax, %bx                   # 光标位置存入基址寄存器bx

    # 栈中获取待打印的字符。返回地址占4字节，pusha的通用寄存器占8*4=32字节
    # 所以第一个参数在esp+36处
    movl 36(%esp), %ecx
    cmpb $0xd, %cl
    jz is_carriage_return           # \r 回车CR，无论是\r或\n都当做CRLF处理
    cmpb $0xa, %cl
    jz is_line_feed                 # \n 换行LF，无论是\r或\n都当做CRLF处理
    cmpb $0x8, %cl
    jz is_backspace                 # \b 退格BS
    jmp put_other
    jmp put_char_set_cursor

is_backspace:                       # 原理：光标坐标-1，位置写入空格（P.S.目前超过左上角情况没有处理）
    decw %bx                        # 光标坐标-1
    shlw $1, %bx                    # 一个字符是2字节，低位显示字符，高位显示特性，光标*2找到对应显存位置
    # movb $0x20, %gs:(%bx)           # 显存低字节显示空格
    # incw %bx
    # movb $0x07, %gs:(%bx)           # 显存高字节写0x7，黑屏白字
    movw $0x0720, %gs:(%bx)
    shrw $1, %bx
    jmp put_char_set_cursor

put_other:                          # 原理：显示可见字符，光标坐标+1
    shlw $1, %bx
    movb %cl, %gs:(%bx)             # 显示可见字符
    incw %bx
    movb $0x07, %gs:(%bx)
    shrw $1, %bx
    incw %bx                        # 光标坐标+1
    cmpw $2000, %bx                 # 光标坐标是右下角80*25=2000，继续执行换行和滚屏
    jl put_char_set_cursor

is_line_feed:
is_carriage_return:                 # 原理：光标回到行首
    xorw %dx, %dx
    movw %bx, %ax
    movw $80, %si
    divw %si
    subw %dx, %bx                   # bx = bx - bx % 80
    addw $80, %bx                   # 光标+80
    cmpw $2000, %bx                 # 如果光标超过右下角，则滚屏
    jl put_char_set_cursor

roll_screen:                        # 滚屏一行
# 将1~24行内容搬到0~23行，%ds:(%esi)->%es:(%edi)
    cld
    movl $960, %ecx
    movl $0xc00b80a0, %esi
    movl $0xc00b8000, %edi
    rep movsd                       
# 最后一行填为空格
    movl $3840, %ebx
    movl $80, %ecx
cls:
    movw $0x0720, %gs:(%ebx)
    addl $2, %ebx
    loop cls
# 光标设置为最后一行第一个字符24*80
    movw $1920, %bx

put_char_set_cursor:
    pushl %ebx
    call set_cursor
    addl $4, %esp
put_char_done:
    popal
    ret


##################################################
# 打印字符串，字符串末尾是\0
#   ebx: 字符串地址
#   cl: 栈中取出的打印字符
##################################################
.globl put_str
put_str:
    pushl %ebx
    pushl %ecx
    xorl %ecx, %ecx
    movl 12(%esp), %ebx             # 返回值4字节+ebx 4字节+ecx 4字节
go_on:
    movb (%ebx), %cl                # cl是当前字符
    jcxz str_over
    pushl %ecx
    call put_char
    addl $4, %esp                   # 回收字符串的栈空间
    incl %ebx                       # 取下一个字符
    jmp go_on
str_over:
    popl %ecx
    popl %ebx
    ret


##################################################
# 16进制打印32位int整数
#   put_int_buffer: 数字转字符串的缓存，大端序(P.S.改成小端序会不会更简单点)
#   eax: 栈中取出的数字
##################################################
.globl put_int
put_int:
    pushal
    movl %esp, %ebp
    movl 36(%ebp), %eax             # 返回值4字节+pushal的32字节
    movl %eax, %edx
    movl $7, %edi                   # put_int_buffer偏移量，开始是最右边7
    movl $8, %ecx
    movl $put_int_buffer, %ebx

# 32位数字，按十六进制形式，从低位向高位逐4位处理，总共循环8次
convert_int:
    andl $0x0000000F, %edx          # dl取低四位
    cmpl $9, %edx
    jg is_A2F
    addl $'0', %edx                 # 如果是0-9，ascii+'0'
    jmp store
is_A2F:
    subl $10, %edx                  # 如果是a-f，ascii-10+'A'
    addl $'A', %edx
store:
    movb %dl, (%ebx, %edi)          # 数字存储到缓冲区put_int_buffer
    decl %edi
    shrl $4, %eax
    movl %eax, %edx
    loop convert_int

# 找到put_int_buffer第一个非0下标，记录在edi中
ready_to_print:
    xorl %edi, %edi
skip_prefix_0:
    cmpl $8, %edi
    je full0
go_on_skip:
    movb (%ebx, %edi), %cl
    incl %edi
    cmpb $'0', %cl
    je skip_prefix_0
    decl %edi
    jmp put_each_num

full0:
    movb $'0', %cl                  # 数字全0打印0
put_each_num:
    pushl %ecx                      # cl保存打印的字符
    call put_char                   # 打印cl
    addl $4, %esp                   # 出栈
    incl %edi                       # edi指向下一个字符
    movb (%ebx, %edi), %cl
    cmpl $8, %edi
    jl put_each_num
    popal
    ret
