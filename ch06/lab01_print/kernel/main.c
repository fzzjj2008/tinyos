#include "print.h"

int main(void)
{
    int pos = 0;
    pos = get_cursor();
    set_cursor(0);
    put_str("I am kernel\n");
    put_str("12\b3\r");
    put_int(0);
    put_char('\n');
    put_int(0xff);
    put_char('\n');
    put_int(0x12345678);
    put_char('\n');
    put_int(0x00000000);
    while (1);
    return 0;
}
