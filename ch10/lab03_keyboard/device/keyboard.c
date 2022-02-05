#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"

#define KBD_BUF_PORT 0x60

static void init_keyboard_handler(void) {
    uint8_t scancode = inb(KBD_BUF_PORT);
    put_int(scancode);
}

void keyboard_init(void) {
    put_str("Keyboard init start\n");
    register_handler(0x21, init_keyboard_handler);
    put_str("Keyboard init done\n");
}
