.code32
.section .text
.globl switch_to
switch_to:
    push %esi
    push %edi
    push %ebx
    push %ebp
    movl 20(%esp), %eax     # 获取参数cur，eax保存当前进程PCB的self_kstack地址
    movl %esp, (%eax)       # 当前栈顶esp保存到当前进程PCB的self_kstack

    movl 24(%esp), %eax     # 获取参数next，eax保存下一进程PCB的self_kstack地址
    movl (%eax), %esp       # next线程栈指针恢复到esp

    pop %ebp
    pop %ebx
    pop %edi
    pop %esi
    ret                     # 用ret改EIP
