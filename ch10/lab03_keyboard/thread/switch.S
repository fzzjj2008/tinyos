.code32
.section .text
.globl switch_to
switch_to:
    push %esi
    push %edi
    push %ebx
    push %ebp
    movl 20(%esp), %eax
    movl %esp, (%eax)

    movl 24(%esp), %eax
    movl %esp, (%eax)

    pop %ebp
    pop %ebx
    pop %edi
    pop %esi
    ret
