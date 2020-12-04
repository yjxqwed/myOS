#include <driver/kb.h>
#include <arch/x86.h>
#include <common/utils.h>
#include <string.h>
#include <kprintf.h>
#include <sys/isr.h>
#include <common/types.h>

// keyboard data port
#define KB_DATA_PORT 0x60
// keyboard control port
#define KB_CTRL_PORT 0x64

#define KB_BREAK_MASK 0x80

#define EXT_CODE 0xe0

/* escapes characters */
#define esc       '\x1b'
#define backspace '\b'
#define tab       '\t'
#define enter     '\n'
#define delete    '\x7f'

/* control characters are invisable */
#define char_invisible ('\0')
#define ctrl_l_char    char_invisible
#define ctrl_r_char    char_invisible
#define shift_l_char   char_invisible
#define shift_r_char   char_invisible
#define alt_l_char     char_invisible
#define alt_r_char     char_invisible
#define caps_lock_char char_invisible

/* make and break codes */
#define shift_l_make    0x2a
#define shift_l_break   (shift_l_make | KB_BREAK_MASK)
#define shift_r_make    0x36
#define shift_r_break   (shift_r_make | KB_BREAK_MASK)
#define alt_l_make      0x38
#define alt_l_break     (alt_l_make | KB_BREAK_MASK)
#define alt_r_make      0xe038
#define alt_r_break     (alt_r_make | KB_BREAK_MASK)
#define ctrl_l_make     0x1d
#define ctrl_l_break    (ctrl_l_make | KB_BREAK_MASK)
#define ctrl_r_make     0xe01d
#define ctrl_r_break    (ctrl_r_make | KB_BREAK_MASK)
#define caps_lock_make  0x3a
#define caps_lock_break (caps_lock_make | KB_BREAK_MASK)

/* index is the make-code */
static char keymap[][2] = {
/* make-code {no-shift, with-shift} */
/* ---------------------------------- */
/* 0x00 */ {'\0', '\0'},
/* 0x01 */ {esc, esc},
/* 0x02 */ {'1', '!'},
/* 0x03 */ {'2', '@'},
/* 0x04 */ {'3', '#'},
/* 0x05 */ {'4', '$'},
/* 0x06 */ {'5', '%'},
/* 0x07 */ {'6', '^'},
/* 0x08 */ {'7', '&'},
/* 0x09 */ {'8', '*'},
/* 0x0A */ {'9', '('},
/* 0x0B */ {'0', ')'},
/* 0x0C */ {'-', '_'},
/* 0x0D */ {'=', '+'},
/* 0x0E */ {backspace, backspace},
/* 0x0F */ {tab, tab},
/* 0x10 */ {'q', 'Q'},
/* 0x11 */ {'w', 'W'},
/* 0x12 */ {'e', 'E'},
/* 0x13 */ {'r', 'R'},
/* 0x14 */ {'t', 'T'},
/* 0x15 */ {'y', 'Y'},
/* 0x16 */ {'u', 'U'},
/* 0x17 */ {'i', 'I'},
/* 0x18 */ {'o', 'O'},
/* 0x19 */ {'p', 'P'},
/* 0x1A */ {'[', '{'},
/* 0x1B */ {']', '}'},
/* 0x1C */ {enter, enter},
/* 0x1D */ {ctrl_l_char, ctrl_l_char},
/* 0x1E */ {'a', 'A'},
/* 0x1F */ {'s', 'S'},
/* 0x20 */ {'d', 'D'},
/* 0x21 */ {'f', 'F'},
/* 0x22 */ {'g', 'G'},
/* 0x23 */ {'h', 'H'},
/* 0x24 */ {'j', 'J'},
/* 0x25 */ {'k', 'K'},
/* 0x26 */ {'l', 'L'},
/* 0x27 */ {';', ':'},
/* 0x28 */ {'\'', '"'},
/* 0x29 */ {'`', '~'},
/* 0x2A */ {shift_l_char, shift_l_char},
/* 0x2B */ {'\\', '|'},
/* 0x2C */ {'z', 'Z'},
/* 0x2D */ {'x', 'X'},
/* 0x2E */ {'c', 'C'},
/* 0x2F */ {'v', 'V'},
/* 0x30 */ {'b', 'B'},
/* 0x31 */ {'n', 'N'},
/* 0x32 */ {'m', 'M'},
/* 0x33 */ {',', '<'},
/* 0x34 */ {'.', '>'},
/* 0x35 */ {'/', '?'},
/* 0x36 */ {shift_r_char, shift_r_char},
/* 0x37 */ {'*', '*'},  // keypad *
/* 0x38 */ {alt_l_char, alt_l_char},
/* 0x39 */ {' ', ' '},
/* 0x3A */ {caps_lock_char, caps_lock_char}
/* other chars are omitted for now */
};

static void kb_handler(isrp_t *p) {
    static bool_t ctrl_down = False;
    static bool_t shift_down = False;
    static bool_t alt_down = False;
    static bool_t caps_locked = False;
    static bool_t ext_code = False;


    uint16_t scan_code = inportb(KB_DATA_PORT);

    if (scan_code == EXT_CODE) {
        ext_code = True;
        return;
    }

    if (ext_code) {
        scan_code |= 0xe000;
        ext_code = False;
    }

    if (scan_code & KB_BREAK_MASK) {
        // bit 7 set => break code
        if (scan_code == shift_l_break || scan_code == shift_r_break) {
            shift_down = False;
        } else if (scan_code == ctrl_l_break || scan_code == ctrl_r_break) {
            ctrl_down = False;
        } else if (scan_code == alt_l_break || scan_code == alt_r_break) {
            alt_down = False;
        }
        // break of other keys are unimportant
        return;
    } else if (
        (scan_code >= 0x0 && scan_code <= 0x3a) ||
        scan_code == alt_r_make ||
        scan_code == ctrl_r_make
    ) {
        // only handle characters in our keymap plus right alt and right ctrl

        if (scan_code == shift_l_make || scan_code == shift_r_make) {
            shift_down = True;
        } else if (scan_code == ctrl_l_make || scan_code == ctrl_r_make) {
            ctrl_down = True;
        } else if (scan_code == alt_l_make || scan_code == alt_r_make) {
            alt_down = True;
        } else if (scan_code == caps_lock_make) {
            caps_locked = !caps_locked;
        } else {
            int shift = 0;
            if (
                (scan_code >= 0x10 && scan_code <= 0x19) ||  // q - p
                (scan_code >= 0x1e && scan_code <= 0x26) ||  // a - l
                (scan_code >= 0x2c && scan_code <= 0x32)     // z - m
            ) {
                // caps_lock only influence letters
                shift = (shift_down ^ caps_locked) ? 1 : 0;
            } else {
                shift = shift_down ? 1 : 0;
            }
            char c = keymap[scan_code & 0xff][shift];
            if (c != '\0') {
                kprintf(KPL_DUMP, "%c", c);
            }
        }
    } else {
        kprintf(KPL_DEBUG, "[Unknown key down: %x]", scan_code);
    }
}

void kb_init() {
    register_handler(INT_KB, kb_handler);
}
