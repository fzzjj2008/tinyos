#include "global.h"
#include "debug.h"
#include "string.h"

/**
 * 注意：同C类库同名函数，在Makefile要指定-fno-builtin
 */
void memset(void* address, uint8_t value, uint32_t size) {
    ASSERT(address != NULL);

    uint8_t* addr = (uint8_t*) address;

    while (size-- > 0) {
        *addr++ = value;
    }
}

void memcpy(void* dst, const void* src, uint32_t size) {
    ASSERT(dst != NULL && src != NULL);

    uint8_t* _dst = (uint8_t*) dst;
    const uint8_t* _src = (uint8_t*) src;

    while (size-- > 0) {
        *_dst++ = *_src++;
    }
}

int memcmp(const void* left, const void* right, uint32_t size) {
    ASSERT(left != NULL && right != NULL);

    const uint8_t* _left = (uint8_t*) left;
    const uint8_t* _right = (uint8_t*) right;

    while (size-- > 0) {
        if (*_left != *_right) {
            return (*_left > *_right ? 1 : -1);
        }
        _left++;
        _right++;
    }
    return 0;
}

char* strcpy(char* dst, const char* src) {
    ASSERT(dst != NULL && src != NULL);

    char* head = dst;
    while ((*dst++ = *src++));
    return head;
}

uint32_t strlen(const char* str) {
    ASSERT(str != NULL);

    uint32_t count = 0;

    while (*str++) {
        ++count;
    }
    return count;
}

int8_t strcmp(const char* left, const char* right) {
    ASSERT(left != NULL && right != NULL);

    while (*left != 0 && *left == *right) {
        ++left;
        ++right;
    }
    return (*left < *right ? -1 : *left > *right);
}

char* strchr(const char* str, const uint8_t c) {
    ASSERT(str != NULL);

    uint8_t item;

    while ((item = *str) != 0) {
        if (item == c) {
            return (char*) str;
        }
        ++str;
    }
    return NULL;
}

/* 从后往前查找字符串str中首次出现字符ch的地址(不是下标,是地址) */
char* strrchr(const char* str, const uint8_t ch) {
    ASSERT(str != NULL);
    const char* last_char = NULL;

    /* 从头到尾遍历一次,若存在ch字符,last_char总是该字符最后一次出现在串中的地址(不是下标,是地址)*/
    while (*str != 0) {
        if (*str == ch) {
        last_char = str;
        }
        str++;
    }
    return (char*)last_char;
}

char* strcat(char* dst, const char* src) {
    ASSERT(dst != NULL && src != NULL);

    char* head = dst;

    while (*dst++);
    --dst;
    while ((*dst++ = *src++));
    return head;
}

/**
 * 统计制定的字符在字符串中出现的次数
 */
uint32_t strchrs(const char* str, const uint8_t c) {
    ASSERT(str != NULL);

    uint32_t result = 0;
    char item;

    while ((item = *str) != 0) {
        if (item == c) {
            ++result;
        }
        ++str;
    }
    return result;
}
