.code32
.section .text
.set SELECTOR_VIDEO,    0x18        # video segment selector, index=0x3, ti=0, rpl=00

##################################################
# 打印字符
##################################################
.globl put_char
put_char:
    pushal
    movw SELECTOR_VIDEO, %ax
    movw %ax, %gs

    movw $0x03d4, %dx
    movb 0x0e, %al
    outb %al, %dx
    movw $0x03d5, %dx
    inb %dx, %al
    movb %al, %ah

    movw $0x03d4, %dx
    movb $0x0f, %al
    outb %al, %dx
    movw $0x03d5, %dx
    inb %dx, %al

    movw %ax, %bx
    movl 36(%esp), %ecx
    cmpb $0xd, %cl
    jz is_carriage_return
    cmpb $0xa, %cl
    jz is_line_feed
    cmpb $0x8, %cl
    jz is_backspace
    jmp put_other

is_backspace:
    decw %bx
    shlw $1, %bx
    movb $0x20, %gs:(%bx)
    incw %bx
    movb $0x07, %gs:(%bx)
    shrw $1, %bx
    jmp set_cursor

put_other:
    shlw $1, %bx
    movb %cl, %gs:(%bx)
    incw %bx
    movb $0x07, %gs:(%bx)
    shrw $1, %bx
    incw %bx
    cmpw $2000, %bx
    jl set_cursor

is_line_feed:
is_carriage_return:
    xorw %dx, %dx
    movw %bx, %ax
    movw $80, %si
    divw %si
    subw %dx, %bx

is_carriage_return_end:
    addw $80, %bx
    cmpw $2000, %bx
is_line_feed_end:
    jl set_cursor

roll_screen:
    cld
    movl $960, %ecx
    movl $0xc00b80a0, %esi
    movl $0xc00b8000, %edi
    rep movsd

    movl $3840, %ebx
    movl $80, %ecx

cls:
    movw $0x0720, %gs:(%ebx)
    addl $2, %ebx
    loop cls
    movw $1920, %bx

set_cursor:
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

put_char_done:
    popal
    ret

##################################################
# 打印字符串
##################################################
.globl put_str
put_str:
    pushl %ebx
    pushl %ecx
    xorl %ecx, %ecx
    movl 12(%esp), %ebx
go_on:
    movb (%ecx), %cl
    cmp $0, %cl
    jz str_over
    pushl %ecx
    call put_char
    addl $4, %esp
    incl %ebx
    jmp go_on
str_over:
    popl %ecx
    popl %ebx
    ret

##################################################
# 打印整数
##################################################
.globl put_int
put_int:
    pushal
    movl %esp, %ebp
    movl 36(%ebp), %eax
    movl %eax, %edx
    movl $7, %edi
    movl $8, %ecx
    movl put_int_buffer, %ebx
is_16based_4bits:
    andl $0x0000000F, %edx
    cmpl $9, %edx
    jg is_A2F
    addl '0', %edx
    jmp store
is_A2F:
    subl $10, %edx
    addl 'A', %edx
store:
    movb %dl, (%ebx, %edi)
    decl %edi
    shrl $4, %eax
    movl %eax, %edx
    loop is_16based_4bits

ready_to_print:
    incl %edi
skip_prefix_0:
    cmpl $8, %edi
    je full0
go_on_skip:
    movb put_int_buffer(%edi), %cl
    incl %edi
    cmpb '0', %cl
    je skip_prefix_0
    decl %edi
    jmp put_each_num

full0:
    movb '0', %cl
put_each_num:
    pushl %ecx
    call put_char
    addl $4, %esp
    incl %edi
    movb put_int_buffer(%edi), %cl
    cmpl $8, %edi
    jl put_each_num
    popal
    ret


.section .data
put_int_buffer:
    .int 0
