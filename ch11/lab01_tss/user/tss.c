#include "tss.h"
#include "global.h"
#include "string.h"
#include "print.h"

static struct tss tss;

/**
 * 模仿Linux任务切换方式，将TSS中的esp0更新为给定线程的0级栈
 */
void update_tss_esp(struct task_struct* pthread) {
    tss.esp0 = (uint32_t*) ((uint32_t) pthread + PAGE_SIZE);
}

/**
 * 构造GDT描述符
 */
static struct gdt_desc make_gdt_desc(uint32_t* desc_addr, uint32_t limit, uint8_t attr_low, uint8_t attr_high) {
    uint32_t desc_base = (uint32_t) desc_addr;
    struct gdt_desc desc;
    desc.limit_low_word = limit & 0x0000ffff;
    desc.base_low_word = desc_base & 0x0000ffff;
    desc.base_mid_byte = ((desc_base & 0x00ff0000) >> 16);
    desc.attr_low_byte = (uint8_t) attr_low;
    desc.limit_high_attr_byte = (((limit & 0x000f0000) >> 16) + (uint8_t) (attr_high));
    desc.base_high_byte = desc_base >> 24;
    return desc;
}

/**
 * 在GDT中创建TSS并加载之
 */
void tss_init() {
    put_str("tss_init start\n");

    uint32_t tss_size = sizeof(tss);
    memset(&tss, 0, tss_size);

    tss.ss0 = SELECTOR_K_STACK;
    tss.io_base = tss_size;

    // gdt的第4个位置添加tss, 即0x900+0x20
    *((struct gdt_desc*) 0xc0000920) = make_gdt_desc((uint32_t*) &tss, tss_size - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);

    // gdt的第5个位置添加dpl为3的代码段
    *((struct gdt_desc*) 0xc0000928) = make_gdt_desc((uint32_t*) 0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    // gdt的第6个位置添加dpl为3的数据段
    *((struct gdt_desc*) 0xc0000930) = make_gdt_desc((uint32_t*) 0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);

    // 重新加载gdt，总共是7个描述符
    uint64_t gdt_operand = ((8 * 7 - 1) | ((uint64_t) (uint32_t) 0xc0000900 << 16));

    asm volatile ("lgdt %0" : : "m" (gdt_operand));
    asm volatile ("ltr %w0" : : "r" (SELECTOR_TSS));
    
    put_str("tss_init done\n");
}
