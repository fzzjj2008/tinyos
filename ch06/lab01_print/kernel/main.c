#include "print.h"

int main(void)
{
    int pos = 0;

    put_str("I am kernel\n");
    put_str("12\b3\r");
    put_int(0);
    put_char('\n');
    put_int(0xff);
    put_char('\n');
    put_int(0x12345678);
    put_char('\n');
    put_int(0x00000000);

    set_cursor(1999);
    put_char('E');
    pos = get_cursor();
    put_int(pos);                   //0x780

    while (1);
    return 0;
}
