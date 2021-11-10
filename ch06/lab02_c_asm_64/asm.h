#ifndef _ASM_H
#define _ASM_H

extern void c_print(int ret);
int asm_sub(int a, int b);
int c_sub(int a, int b)
{
    c_print(a - b);
    return a - b;
}

#endif  /*_ASM_H*/
