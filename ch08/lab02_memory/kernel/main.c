#include "print.h"
#include "init.h"
#include "string.h"
#include "memory.h"

void test_str()
{
    char* str1 = "123\n";
    char str2[16];
    int ret;
    char* addr;

    memset(str2, 0, sizeof(str2));
    put_str(str2);                  // str2 = ""
    memcpy(str2, str1, 4);
    put_str(str1);                  // str1 = "123\n"
    put_str(str2);                  // str2 = "123\n"

    ret = memcmp(str1, str2, sizeof(str1));
    put_int(ret);                   // ret = 0
    put_char('\n');

    strcpy(str1, "abc\n");
    strcpy(str1, "123\n");
    ret = strcmp(str1, str2);
    put_str(str1);                  // str1 = "abc\n"
    put_str(str2);                  // str2 = "123\n"    
    put_int(ret);                   // ret = 1
    put_char('\n');

    ret = strlen(str1);
    put_int(ret);                   // ret = 4
    put_char('\n');

    strcpy(str1, "abc\n");
    strcpy(str2, "bcd");
    strcat(str2, str1);
    put_str(str2);                  // str2 = "bcdabc\n"
    addr = strchr(str2, 'a');
    put_int((uint32_t) addr);       // addr
    put_char('\n');
    ret = strchrs(str2, 'c');
    put_int(ret);                   // ret = 2
    put_char('\n');
}

void test_bitmap()
{
    uint32_t addr = 0;
    struct bitmap btmp;
    int idx;

    btmp.btmp_bytes_len = sizeof(addr);
    btmp.bits = (uint8_t*) &addr;
    bitmap_init(&btmp);
    put_int(addr);                          // addr = 0
    put_char('\n');

    put_int(bitmap_get(&btmp, 5));          // 0
    put_char('\n');
    bitmap_set(&btmp, 5, 1);
    put_int(bitmap_get(&btmp, 5));          // 1
    put_char('\n');
    put_int(addr);                          // addr = 0x20 (00000000_00000000_00000000_00100000)
    put_char('\n');

    idx = bitmap_scan(&btmp, 6);
    put_int(idx);                           // idx = 6
    put_char('\n');

    idx = bitmap_scan(&btmp, 5);
    put_int(idx);                           // idx = 0
    put_char('\n');

    idx = bitmap_scan(&btmp, 30);
    put_int(idx);                           // idx = -1
    put_char('\n');
}

int main(void)
{
    put_str("kernel_init\n");
    init_all();

    // test_str();
    // test_bitmap();

    void* vaddr = get_kernel_pages(3);
    put_str("\nstart vaddr: ");
    put_int((uint32_t) vaddr);
    put_char('\n');

    while (1);
    return 0;
}
