#include <stdio.h>

void hello1()
{
    char* hello_str = "hello world\n";
    int count = 0;
    // 普通内联汇编
    __asm__("movl $4, %eax\n\t"             \
            "movl $1, %ebx\n\t"             \
            "movl hello_str, %ecx\n\t"      \
            "movl $13, %edx\n\t"            \
            "int $0x80\n\t"                 \
            "movl %eax, count"              \
    );
    printf("The number of bytes written is %d\n", count);
}

void hello2()
{
    int ret_cnt = 0, test = 0;
    char* fmt = "hello, world\n";
    // 扩展内联汇编，printf输出结果放在eax中
    // 由于movl $6, %2会使用test变量，gcc随机使用eax寄存器可能会把输出结果破坏
    // 所以约束&表示output的寄存器只能给output用，不给input用
    __asm__("pushl %1\n\t"                  \
            "call printf"                   \
            "addl $4, %%esp"                \
            "movl $6, %2"                   \
            :"=&a"(ret_cnt)                 \
            :"m"(fmt), "r"(test)            \
    );
    printf("The number of bytes written is %d\n", ret_cnt);
}

int add1(int in_a, int in_b)
{
    int result;
    // 内联汇编，addl %ebx, %eax，返回值给eax
    __asm__("addl %%ebx, %%eax" : "=a"(result) : "a"(in_a), "b"(in_b));
    return result;
}

int add2(int in_a, int in_b)
{    
    int result;
    // 序号占位符
    // %0表示result, %1表示in_a, %2表示in_b
    __asm__("addl %2, %1" : "=a"(result) : "a"(in_a), "b"(in_b));
    return result;
}

int add3(int in_a, int in_b)
{
    int out;
    // [result]是名称占位符
    __asm__("addl %[addend], %1\n\t"        \
            "movb %%al, %[result]"          \
        :[result]"=m"(out)                  \
        :"a"(in_a), [addend]"b"(in_b)       \
    );
    return out;
}

void add4(int in_a, int in_b)
{
    // +a操作数可读可写, eax作为输出
    __asm__("addl %%ebx, %%eax" : "+a"(in_a) : "b"(in_b));
}

void add5(int in_a)
{
    // 使用约束0表示in_a和sum使用相同的寄存器，即输入输出使用eax
    // I表示立即数约束, 2变成$2
    // %I表示I对应的操作数可以和下一个输入约束的操作数对换位置
    __asm__("addl %1, %0" : "=a"(sum) : "%I"(2), "0"(in_a));
    return sum;
}

void mov1(int in_a, int *in_b)
{
    // mov 0x8(%ebp),%eax
    // mov %al,0xc(%ebp)
    // b0是表示in_a低8位，前缀为b是防止movb报错
    __asm__("movb %b0, %1"::"a"(in_a), "m"(in_b));
}

void mov2(int in_a, int *in_b)
{
    // h1是表示in_a高8位
    __asm__("movb %h1, %0"::"m"(in_b), "a"(in_a));
}


int main(int argc, char* argv[])
{
    int sum = 0;
    int in_a = 1, in_b = 2;

    // 打印hello
    hello1();
    hello2();

    // sum = in_a + in_b
    sum = add1(in_a, in_b);
    // sum = add2(in_a, in_b);
    // sum = add3(in_a, in_b);
    // sum = add4(in_a, in_b);
    // sum = add5(in_a);
    printf("sum=%d, in_a=%d, in_b=%d\n", sum, in_a, in_b);

    // mov in_a -> in_b
    in_a = 0x12345678; in_b = 0;
    mov1(in_a, &in_b);
    // mov2(in_a, &in_b);
    printf("in_a=%x, in_b=%x\n", in_a, in_b);

    return 0;
}
