.code32
.section .text
.globl switch_to
switch_to:
    push %esi
    push %edi
    push %ebx
    push %ebp
    movl 20(%esp), %eax     # cur线程栈顶指针esp保存到cur线程PCB中的self_kstack
    movl %esp, (%eax)

    movl 24(%esp), %eax     # next线程栈顶指针esp保存到next线程PCB中的self_kstack
    movl %esp, (%eax)

    pop %ebp
    pop %ebx
    pop %edi
    pop %esi
    ret
