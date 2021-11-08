char* str = "hello world\n";
int count = 0;

void hello()
{
    __asm__("                       \
        pusha;                      \
        movl $4, %eax;              \
        movl $1, %ebx;              \
        movl str, %ecx;             \
        movl $12, %edx;             \
        int $0x80;                  \
        mov %eax, count;            \
        popa");
}

int add(int in_a, int in_b)
{
    int out_sum;
    __asm__("addl %%ebx, %%eax;"    \
        :"=a"(out_sum)              \
        :"a"(in_a), "b"(in_b));
    __asm__("addl %2, %1;"          \
        :"=a"(out_sum)              \
        :"a"(in_a), "b"(in_b));
    return out_sum;
}

void mem(int in_a, int in_b)
{
    // 内联汇编，al写入in_b的内存
    __asm__("movb %b0, %1;"         \
        ::"a"(in_a), "m"(in_b));
}

int main()
{
    int sum;

    hello();
    sum = add(1, 2);
    mem(1, 2);

    while(1);
    return 0;
}
