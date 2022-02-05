#include "bitmap.h"
#include "stdint.h"
#include "string.h"
#include "print.h"
#include "interrupt.h"
#include "debug.h"

void bitmap_init(struct bitmap* btmap) {
    memset(btmap->bits, 0, btmap->btmp_bytes_len);
}

/**
 * index位为1返回非0，否则返回0
 */
int bitmap_get(struct bitmap* btmap, uint32_t index) {
    uint32_t byte_index = index / 8;
    uint32_t bit_odd = index % 8;

    return (btmap->bits[byte_index] & (BITMAP_MASK << bit_odd));
}

/**
 * 在位图bitmap中申请连续的cnt个位
 * P.S.可以优化
 */
int bitmap_scan(struct bitmap* btmap, uint32_t cnt) {
    uint32_t idx_byte = 0;

    // 以字节为单位进行查找，若没有找到返回-1
    while ((0xff == btmap->bits[idx_byte]) && idx_byte < btmap->btmp_bytes_len) {
        ++idx_byte;
    }
    if (idx_byte == btmap->btmp_bytes_len) {
        return -1;
    }

    // 找第一个非0xff字节，定位bit为0的起始位bit_idx_start
    int idx_bit = 0;
    while ((uint8_t) (BITMAP_MASK << idx_bit) & btmap->bits[idx_byte]) {
        ++idx_bit;
    }
    int bit_idx_start = (idx_byte * 8 + idx_bit);

    // 如果只判断长度为1的字符串，直接返回
    if (cnt == 1) {
        return bit_idx_start;
    }

    uint32_t bit_left = (btmap->btmp_bytes_len * 8 - bit_idx_start);
    uint32_t count = 1;
    uint32_t next_bit = bit_idx_start + 1;

    // 找连续的cnt个位。如果有连续返回下标；没有连续的返回-1
    bit_idx_start = -1;
    while (bit_left-- > 0) {
        if (!(bitmap_get(btmap, next_bit))) {
            ++count;
        } else {
            count = 0;
        }

        if (count == cnt) {
            bit_idx_start = (next_bit - cnt + 1);
            break;
        }

        next_bit++;
    }

    return bit_idx_start;
}

void bitmap_set(struct bitmap* btmap, uint32_t index, int8_t value) {
    ASSERT(value == 0 || value == 1);

    uint32_t byte_index = index / 8;
    uint32_t bit_odd = index % 8;

    if (value) {
        btmap->bits[byte_index] |= (BITMAP_MASK << bit_odd);
    } else {
        btmap->bits[byte_index] &= ~(BITMAP_MASK << bit_odd);
    }
}
