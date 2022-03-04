.code32
.extern main
.section .text

.globl _start
_start:
    pushl %ebx
    pushl %ecx
    call main
