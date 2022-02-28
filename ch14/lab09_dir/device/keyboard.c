#include "keyboard.h"
#include "print.h"
#include "io.h"
#include "interrupt.h"
#include "global.h"
#include "ioqueue.h"

#define KEYBOARD_BUF_PORT 0x60

// 用转义字符定义的控制字符
#define ESC         '\x1b'          // 书这里用八进制'\033'，为代码可读性改为16进制
#define BACKSPACE   '\b'
#define TAB         '\t'
#define ENTER       '\r'

// 不可见字符定义为0
#define CHAR_INVISIBLE  0
#define CTRL_L_CHAR     CHAR_INVISIBLE
#define CTRL_R_CHAR     CHAR_INVISIBLE
#define SHIFT_L_CHAR    CHAR_INVISIBLE
#define SHIFT_R_CHAR    CHAR_INVISIBLE
#define ALT_L_CHAR      CHAR_INVISIBLE
#define ALT_R_CHAR      CHAR_INVISIBLE
#define CAPS_LOCK_CHAR  CHAR_INVISIBLE

// 控制字符的通码和断码
#define SHIFT_L_MAKE    0x2a
#define SHIFT_R_MAKE    0x36
#define ALT_L_MAKE      0x38
#define ALT_R_MAKE      0xe038
#define ALT_R_BREAK     0xe0b8
#define CTRL_L_MAKE     0x1d
#define CTRL_R_MAKE     0xe01d
#define CTRL_R_BREAK    0xe09d
#define CAPS_LOCK_MAKE  0x3a

/**
 * 记录以下按键是否已被按下
 */
static bool ctrl_pressed, shift_pressed, alt_pressed, caps_lock_pressed, ext_scan_code;
static struct ioqueue keyboard_buffer;
static void keyboard_display_control_key();

/**
 * 以通码为索引的显示字符数组，零号元素为shift没有按下时的展示，1反之
 */
static char keymap[][2] = {             // 第一套扫描码，见表10-1. 仅处理00~3A
    {0, 0},                             // 0x00
    {ESC, ESC},                         // 0x01, 对应断码是0x81=0x01+0x80, 以下类似
    {'1', '!'},                         // 0x02
    {'2', '@'},                         // 0x03
    {'3', '#'},                         // 0x04
    {'4', '$'},                         // 0x05
    {'5', '%'},                         // 0x06
    {'6', '^'},                         // 0x07
    {'7', '&'},                         // 0x08
    {'8', '*'},                         // 0x09
    {'9', '('},                         // 0x0a
    {'0', ')'},                         // 0x0b
    {'-', '_'},                         // 0x0c
    {'=', '+'},                         // 0x0d
    {BACKSPACE, BACKSPACE},             // 0x0e
    {TAB, TAB},                         // 0x0f
    {'q', 'Q'},                         // 0x10
    {'w', 'W'},                         // 0x11
    {'e', 'E'},                         // 0x12
    {'r', 'R'},                         // 0x13
    {'t', 'T'},                         // 0x14
    {'y', 'Y'},                         // 0x15
    {'u', 'U'},                         // 0x16
    {'i', 'I'},                         // 0x17
    {'o', 'O'},                         // 0x18
    {'p', 'P'},                         // 0x19
    {'[', '{'},                         // 0x1a
    {']', '}'},                         // 0x1b
    {ENTER, ENTER},                     // 0x1c, 小键盘ENTER是0xe01c
    {CTRL_L_CHAR, CTRL_L_CHAR},         // 0x1d, CTRL_R_CHAR是0xe01d
    {'a', 'A'},                         // 0x1e
    {'s', 'S'},                         // 0x1f
    {'d', 'D'},                         // 0x20
    {'f', 'F'},                         // 0x21
    {'g', 'G'},                         // 0x22
    {'h', 'H'},                         // 0x23
    {'j', 'J'},                         // 0x24
    {'k', 'K'},                         // 0x25
    {'l', 'L'},                         // 0x26
    {';', ':'},                         // 0x27
    {'\'', '"'},                        // 0x28
    {'`', '~'},                         // 0x29
    {SHIFT_L_CHAR, SHIFT_L_CHAR},       // 0x2a
    {'\\', '|'},                        // 0x2b
    {'z', 'Z'},                         // 0x2c
    {'x', 'X'},                         // 0x2d
    {'c', 'C'},                         // 0x2e
    {'v', 'V'},                         // 0x2f
    {'b', 'B'},                         // 0x30
    {'n', 'N'},                         // 0x31
    {'m', 'M'},                         // 0x32
    {',', '<'},                         // 0x33
    {'.', '>'},                         // 0x34
    {'/', '?'},                         // 0x35
    {SHIFT_R_CHAR, SHIFT_R_CHAR},       // 0x36
    {'*', '*'},                         // 0x37
    {ALT_L_CHAR, ALT_L_CHAR},           // 0x38, ALT_R_CHAR是0xe038
    {' ', ' '},                         // 0x39, Space
    {CAPS_LOCK_CHAR, CAPS_LOCK_CHAR}    // 0x3a
};

/**
 * 键盘中断处理
 */
static void intr_keyboard_handler(void) {
    bool shift_pressed_last = shift_pressed;
    bool caps_lock_pressed_last = caps_lock_pressed;
    uint16_t code = inb(KEYBOARD_BUF_PORT);

    // e0开头为扩展扫描码，等待第二个扫描码
    if (code == 0xe0) {
        ext_scan_code = TRUE;
        return;
    }
    if (ext_scan_code) {
        code = ((0xe000) | code);
        ext_scan_code = FALSE;
    }

    // 是否是断码, 0x80是否有置位
    bool break_code = ((code & 0x0080) != 0);
    // 断码处理
    if (break_code) {
        // 获得断码对应的通码
        uint16_t make_code = (code &= 0xff7f);
        // 如果收到CTRL、SHIFT、ALT的断码，认为按键已松开
        // CAPS_LOCK特殊处理，按键松开也有可能caps_lock按下
        if (make_code == CTRL_L_MAKE || make_code == CTRL_R_MAKE) {
            ctrl_pressed = FALSE;
        } else if (make_code == SHIFT_L_MAKE || make_code == SHIFT_R_MAKE) {
            shift_pressed = FALSE;
        } else if (make_code == ALT_L_MAKE || make_code == ALT_R_MAKE) {
            alt_pressed = FALSE;
        }
        // 展示控制键
        keyboard_display_control_key();
        return;
    }

    // 到这里就说明是通码，但是我们只能处理0x01-0x3a之间的按键，再加上两个特殊通码
    if ((code <= 0x00 || code > 0x3a) && (code != ALT_R_MAKE) && (code != CTRL_R_MAKE)) {
        put_str("unknown key\n");
        return;
    }

    // 处理SHIFT组合按键
    bool shift = FALSE;
    if (code < 0x0e || code == 0x29 || code == 0x1a || code == 0x1b || code == 0x2b || code == 0x27 || code == 0x28 || code == 0x33 || code == 0x34 || code == 0x35) {
        // 0-9, [等字符, shift和caps同时按下，不关心caps
        if (shift_pressed_last) {
            shift = TRUE;
        }
    } else {
        if (shift_pressed_last && caps_lock_pressed_last) {
            // shift和caps同时按下，相互抵消
            shift = FALSE;
        } else if (shift_pressed_last || caps_lock_pressed_last) {
            // shift和caps有一个按下，认为SHIFT按下
            shift = TRUE;
        } else {
            // shift和caps均未按下，认为SHIFT未按下
            shift = FALSE;
        }
    }

    uint8_t index = (code & 0x00ff);
    char cur_char = keymap[index][shift];

    if (cur_char && !is_queue_full(&keyboard_buffer)) {
        // 打印可见字符
        // 这里有判断键盘缓冲区满。缓冲区keyboard_buffer大小为64字节，定义在keyboard.h
        put_char(cur_char);
        queue_putchar(&keyboard_buffer, cur_char);
    } else {
        // 处理ctrl、shift、alt、caps_lock
        if (code == CTRL_L_MAKE || code == CTRL_R_MAKE) {
            ctrl_pressed = TRUE;
        } else if (code == SHIFT_L_MAKE || code == SHIFT_R_MAKE) {
            shift_pressed = TRUE;
        } else if (code == ALT_L_MAKE || code == ALT_R_MAKE) {
            alt_pressed = TRUE;
        } else if (code == CAPS_LOCK_MAKE) {
            caps_lock_pressed = !caps_lock_pressed;
        }
    }

    // 展示控制键
    keyboard_display_control_key();
}

static void keyboard_display_control_key() {
    char ctrl_key = (ctrl_pressed ? 'C' : 'c');
    char shift_key = (shift_pressed ? 'S' : 's');
    char alt_key = (alt_pressed ? 'A' : 'a');
    char caps_lock_key = (caps_lock_pressed ? 'L' : 'l');
    uint16_t cur_cursor = get_cursor();
    set_cursor(0);
    put_char(ctrl_key);
    put_char(shift_key);
    put_char(alt_key);
    put_char(caps_lock_key);
    set_cursor(cur_cursor);
}

void keyboard_init(void) {
    put_str("keyboard_init\n");
    ioqueue_init(&keyboard_buffer);
    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard_init done\n");
}
