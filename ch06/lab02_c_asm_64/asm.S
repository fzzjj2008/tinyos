.section .text
##################################################
# asm_sub(int a, int b)
##################################################
.globl asm_sub
asm_sub:
    push %rbp
    mov %rsp, %rbp
    sub $0x10, %rsp
 
    mov %edi, -4(%rbp)      # 第1个参数
    mov %esi, -8(%rbp)      # 第2个参数
    mov -4(%rbp), %eax
    sub -8(%rbp), %eax
 
    # 调用c_print
    mov %eax, %edi
    call c_print

    mov -4(%rbp), %eax
    sub -8(%rbp), %eax
    leave
    ret
