.code32
.section .data
hello_str:
    .asciz "hello world!\n"


.section .text
.type asm_hello, @function
.global asm_hello
asm_hello:
    pushl %ebp
    movl %esp, %ebp
    movl $4, %eax
    movl $1, %ebx
    movl $hello_str, %ecx
    movl $13, %edx
    int $0x80
    popl %ebp
    ret


.type asm_sub, @function
.global asm_sub
asm_sub:
    pushl %ebp
    movl %esp, %ebp
    sub $0x08, %esp
    movl 8(%ebp), %eax
    movl %eax, (%esp)
    movl 12(%ebp), %eax
    movl %eax, 4(%esp)
    call c_sub
    add $0x08, %esp
    popl %ebp
    ret
