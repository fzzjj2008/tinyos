#ifndef _LIB_KERNEL_BITMAP_H
#define _LIB_KERNEL_BITMAP_H

#include "global.h"

#define BITMAP_MASK 1

// bitmap以bit为单位，整体以Byte为单位
struct bitmap {
    uint32_t btmp_bytes_len;
    uint8_t* bits;
};

void bitmap_init(struct bitmap* btmap);
int bitmap_get(struct bitmap* btmap, uint32_t bit_idx);
int bitmap_scan(struct bitmap* btmap, uint32_t cnt);
void bitmap_set(struct bitmap* btmap, uint32_t index, int8_t value);

#endif
