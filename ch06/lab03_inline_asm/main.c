#include <stdio.h>

char* hello_str = "hello world\n";
int count = 0;

void hello()
{
    __asm__("movl $4, %eax\n\t"             \
            "movl $1, %ebx\n\t"             \
            "movl hello_str, %ecx\n\t"      \
            "movl $13, %edx\n\t"            \
            "int $0x80\n\t"                 \
            "movl %eax, count"
    );
}

int add(int in_a, int in_b)
{
    int out_sum;
    // 内联汇编，addl %ebx, %eax，返回值给eax
    __asm__("addl %%ebx, %%eax"             \
            :"=a"(out_sum)                  \
            :"a"(in_a), "b"(in_b)           \
    );
    __asm__("addl %2, %1;"                  \
        :"=a"(out_sum)                      \
        :"a"(in_a), "b"(in_b)               \
    );
    return out_sum;
}

void mem(int in_a, int in_b)
{
    // 内联汇编，movb 
    __asm__("movb %b0, %1"                  \
            ::"a"(in_a), "m"(in_b)          \
    );
}

int main(int argc, char* argv[])
{
    int sum = 0;
    int in_a = 1, in_b = 2;

    // 打印hello
    hello();
    printf("count=%d\n", count);
    // sum = in_a + in_b
    sum = add(in_a, in_b);
    printf("sum=%d, in_a=%d, in_b=%d\n", sum, in_a, in_b);
    // in_a -> in_b
    mem(1, 2);
    printf("in_a=%d, in_b=%d\n", in_a, in_b);

    return 0;
}
