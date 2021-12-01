#ifndef _LIB_IO_H
#define _LIB_IO_H

#include "stdint.h"

/**
 * 向端口写入一个字节
 * port: 0~255, %w1对应dx, N是立即数
 * data: 字节，%b0对应al
 */
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));   
}

/**
 * addr起始的word_cnt个字节写入端口port
 * port: 0~255, 对应edx
 * addr: 地址, ds:esi寄存器
 * word_cnd: 字节数，对应ecx
 */
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {
    asm volatile ("cld; rep outsw" : "+S" (addr), "+c" (word_cnt) : "d" (port));
}

/**
 * 从端口port读一个字节返回
 * port: 0~255, %w1对应dx, N是立即数
 * data: 字节，%b0对应al
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

/**
 * port读取的word_cnt字节写入addr
 * port: 0~255, 对应edx
 * addr: 地址, es:edi寄存器
 * word_cnd: 字节数，对应ecx
 */
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt) {
    asm volatile ("cld; rep insw" : "+D" (addr), "+c" (word_cnt) : "d" (port) : "memory");
}

#endif
