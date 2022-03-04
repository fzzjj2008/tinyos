#ifndef __LIB_KERNEL_PRINT_H
#define __LIB_KERNEL_PRINT_H

#include "stdint.h"
uint16_t get_cursor(void);
void set_cursor(uint16_t pos);
void put_char(const uint8_t char_ascii);
void put_str(const char* message);
void put_int(const uint32_t num);
void cls_screen(void);

#endif  /*__LIB_KERNEL_PRINT_H*/
