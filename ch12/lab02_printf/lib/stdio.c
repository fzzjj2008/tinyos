#include "stdio.h"
#include "global.h"
#include "stdint.h"
#include "string.h"
#include "syscall.h"

#define va_start(ap, v)     ap = (va_list) &v       // 把ap指向第一个固定参数v
#define va_arg(ap, t)       *((t*)(ap += 4))        // ap指向下一个参数并返回其值
#define va_end(ap)          ap = NULL               // 清除ap

/**
 * 将整型转换成字符，base是几进制
 */
static void itoa(uint32_t value, char** buf_ptr_addr, uint8_t base) {
    uint32_t m = value % base;                      // m是最低位
    uint32_t i = value / base;                      // i是去掉最低位的剩余数
    if (i) {
        itoa(i, buf_ptr_addr, base);                // 这里使用递归方式拼接buf_ptr_addr(P.S.可以用非递归优化)
    }
    if (m < 10) {
        *((*buf_ptr_addr)++) = m + '0';
    } else {
        *((*buf_ptr_addr)++) = m - 10 + 'A';
    }
}

/**
 * 将参数ap按照格式format输出到字符串str，并返回替换后str长度
 */
uint32_t vsprintf(char* str, const char* format, va_list ap) {
    char* buf_ptr = str;
    const char* index_ptr = format;
    char index_char = *index_ptr;
    int arg_int;
    char* arg_str;

    // 解析format，将实参转换格式输出到字符串str中
    // P.S.这里仅处理简单的几个类型
    while (index_char) {
        // 如果不是%[?]，则直接打印
        if (index_char != '%') {
            *(buf_ptr++) = index_char;
            index_char = *(++index_ptr);
            continue;
        }
        // index_char是%后面的字符
        index_char = *(++index_ptr);
        switch (index_char) {
            case 's':
                arg_str = va_arg(ap, char*);
                strcpy(buf_ptr, arg_str);
                index_char = *(++index_ptr);
                break;
            case 'c':
                *(buf_ptr++) = va_arg(ap, char);
                index_char = *(++index_ptr);
                break;
            case 'd':
                arg_int = va_arg(ap, int);
                // 负数前面输出一个符号'-'
                if (arg_int < 0) {
                    arg_int = 0 - arg_int;
                    *buf_ptr++ = '-';
                }
                itoa(arg_int, &buf_ptr, 10);
                index_char = *(++index_ptr);
                break;
            case 'x':
                arg_int = va_arg(ap, int);
                itoa(arg_int, &buf_ptr, 16);
                index_char = *(++index_ptr);
                break;
            case '%':
                *buf_ptr++ = '%';
                index_char = *(++index_ptr);
                break;
            default:
                index_char = *(++index_ptr);
                break;
        }
    }

    return strlen(str);
}

/**
 * 格式化输出字符串format至终端
 */
uint32_t printf(const char* format, ...) {
    va_list args;
    char buf[1024] = {0};
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    return write(buf);
}

/**
 * 格式化输出字符串format至buf
 */
uint32_t sprintf(char* buf, const char* format, ...) {
    va_list args;
    uint32_t retval;
    va_start(args, format);
    retval = vsprintf(buf, format, args);
    va_end(args);
    return retval;
}
