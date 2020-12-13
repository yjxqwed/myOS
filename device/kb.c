#include <device/kb.h>
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

// #define KB_BREAK_MASK 0x80

#define EXT_CODE 0xE0

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
#define shift_l_make    0x2A
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
/* make-code {no-shift, with-shift, ext} */
/* ---------------------------------- */
/* 0x00 */ {0, 0},
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
/* 0x3B */
/* other chars are omitted for now */
};

uint16_t keymap1[][3] = {
/* scan-code               !Shift        Shift        E0 XX    */
/* ==================================================================== */
/* 0x00 - none       */    {0,           0,           0},
/* 0x01 - ESC        */    {ESC,         ESC,         0},
/* 0x02 - '1'        */    {'1',         '!',         0},
/* 0x03 - '2'        */    {'2',         '@',         0},
/* 0x04 - '3'        */    {'3',         '#',         0},
/* 0x05 - '4'        */    {'4',         '$',         0},
/* 0x06 - '5'        */    {'5',         '%',         0},
/* 0x07 - '6'        */    {'6',         '^',         0},
/* 0x08 - '7'        */    {'7',         '&',         0},
/* 0x09 - '8'        */    {'8',         '*',         0},
/* 0x0A - '9'        */    {'9',         '(',         0},
/* 0x0B - '0'        */    {'0',         ')',         0},
/* 0x0C - '-'        */    {'-',         '_',         0},
/* 0x0D - '='        */    {'=',         '+',         0},
/* 0x0E - BS         */    {BACKSPACE,   BACKSPACE,   0},
/* 0x0F - TAB        */    {TAB,         TAB,         0},
/* 0x10 - 'q'        */    {'q',         'Q',         0},
/* 0x11 - 'w'        */    {'w',         'W',         0},
/* 0x12 - 'e'        */    {'e',         'E',         0},
/* 0x13 - 'r'        */    {'r',         'R',         0},
/* 0x14 - 't'        */    {'t',         'T',         0},
/* 0x15 - 'y'        */    {'y',         'Y',         0},
/* 0x16 - 'u'        */    {'u',         'U',         0},
/* 0x17 - 'i'        */    {'i',         'I',         0},
/* 0x18 - 'o'        */    {'o',         'O',         0},
/* 0x19 - 'p'        */    {'p',         'P',         0},
/* 0x1A - '['        */    {'[',         '{',         0},
/* 0x1B - ']'        */    {']',         '}',         0},
/* 0x1C - CR/LF      */    {ENTER,       ENTER,       PAD_ENTER},
/* 0x1D - l. Ctrl    */    {CTRL_L,      CTRL_L,      CTRL_R},
/* 0x1E - 'a'        */    {'a',         'A',         0},
/* 0x1F - 's'        */    {'s',         'S',         0},
/* 0x20 - 'd'        */    {'d',         'D',         0},
/* 0x21 - 'f'        */    {'f',         'F',         0},
/* 0x22 - 'g'        */    {'g',         'G',         0},
/* 0x23 - 'h'        */    {'h',         'H',         0},
/* 0x24 - 'j'        */    {'j',         'J',         0},
/* 0x25 - 'k'        */    {'k',         'K',         0},
/* 0x26 - 'l'        */    {'l',         'L',         0},
/* 0x27 - ';'        */    {';',         ':',         0},
/* 0x28 - '\''       */    {'\'',        '"',         0},
/* 0x29 - '`'        */    {'`',         '~',         0},
/* 0x2A - l. SHIFT   */    {SHIFT_L,     SHIFT_L,     0},
/* 0x2B - '\'        */    {'\\',        '|',         0},
/* 0x2C - 'z'        */    {'z',         'Z',         0},
/* 0x2D - 'x'        */    {'x',         'X',         0},
/* 0x2E - 'c'        */    {'c',         'C',         0},
/* 0x2F - 'v'        */    {'v',         'V',         0},
/* 0x30 - 'b'        */    {'b',         'B',         0},
/* 0x31 - 'n'        */    {'n',         'N',         0},
/* 0x32 - 'm'        */    {'m',         'M',         0},
/* 0x33 - ','        */    {',',         '<',         0},
/* 0x34 - '.'        */    {'.',         '>',         0},
/* 0x35 - '/'        */    {'/',         '?',         PAD_SLASH},
/* 0x36 - r. SHIFT   */    {SHIFT_R,     SHIFT_R,     0},
/* 0x37 - '*'        */    {'*',         '*',         0},
/* 0x38 - ALT        */    {ALT_L,       ALT_L,       ALT_R},
/* 0x39 - ' '        */    {' ',         ' ',         0},
/* 0x3A - CapsLock   */    {CAPS_LOCK,   CAPS_LOCK,   0},
/* 0x3B - F1         */    {F1,          F1,          0},
/* 0x3C - F2         */    {F2,          F2,          0},
/* 0x3D - F3         */    {F3,          F3,          0},
/* 0x3E - F4         */    {F4,          F4,          0},
/* 0x3F - F5         */    {F5,          F5,          0},
/* 0x40 - F6         */    {F6,          F6,          0},
/* 0x41 - F7         */    {F7,          F7,          0},
/* 0x42 - F8         */    {F8,          F8,          0},
/* 0x43 - F9         */    {F9,          F9,          0},
/* 0x44 - F10        */    {F10,         F10,         0},
/* 0x45 - NumLock    */    {NUM_LOCK,    NUM_LOCK,    0},
/* 0x46 - ScrLock    */    {SCROLL_LOCK, SCROLL_LOCK, 0},
/* 0x47 - Home       */    {PAD_HOME,    '7',         HOME},
/* 0x48 - CurUp      */    {PAD_UP,      '8',         UP},
/* 0x49 - PgUp       */    {PAD_PAGEUP,  '9',         PAGEUP},
/* 0x4A - '-'        */    {PAD_MINUS,   '-',         0},
/* 0x4B - Left       */    {PAD_LEFT,    '4',         LEFT},
/* 0x4C - MID        */    {PAD_MID,     '5',         0},
/* 0x4D - Right      */    {PAD_RIGHT,   '6',         RIGHT},
/* 0x4E - '+'        */    {PAD_PLUS,    '+',         0},
/* 0x4F - End        */    {PAD_END,     '1',         END},
/* 0x50 - Down       */    {PAD_DOWN,    '2',         DOWN},
/* 0x51 - PgDown     */    {PAD_PAGEDOWN,'3',         PAGEDOWN},
/* 0x52 - Insert     */    {PAD_INS,     '0',         INSERT},
/* 0x53 - Delete     */    {PAD_DOT,     '.',         DELETE},
/* 0x54 - Enter      */    {0,            0,          0},
/* 0x55 - ???        */    {0,            0,          0},
/* 0x56 - ???        */    {0,            0,          0},
/* 0x57 - F11        */    {F11,          F11,        0},
/* 0x58 - F12        */    {F12,          F12,        0},
/* 0x59 - ???        */    {0,            0,          0},
/* 0x5A - ???        */    {0,            0,          0},
/* 0x5B - ???        */    {0,            0,          GUI_L},
/* 0x5C - ???        */    {0,            0,          GUI_R},
/* 0x5D - ???        */    {0,            0,          APPS},
/* 0x5E - ???        */    {0,            0,          0},
/* 0x5F - ???        */    {0,            0,          0},
/* 0x60 - ???        */    {0,            0,          0},
/* 0x61 - ???        */    {0,            0,          0},
/* 0x62 - ???        */    {0,            0,          0},
/* 0x63 - ???        */    {0,            0,          0},
/* 0x64 - ???        */    {0,            0,          0},
/* 0x65 - ???        */    {0,            0,          0},
/* 0x66 - ???        */    {0,            0,          0},
/* 0x67 - ???        */    {0,            0,          0},
/* 0x68 - ???        */    {0,            0,          0},
/* 0x69 - ???        */    {0,            0,          0},
/* 0x6A - ???        */    {0,            0,          0},
/* 0x6B - ???        */    {0,            0,          0},
/* 0x6C - ???        */    {0,            0,          0},
/* 0x6D - ???        */    {0,            0,          0},
/* 0x6E - ???        */    {0,            0,          0},
/* 0x6F - ???        */    {0,            0,          0},
/* 0x70 - ???        */    {0,            0,          0},
/* 0x71 - ???        */    {0,            0,          0},
/* 0x72 - ???        */    {0,            0,          0},
/* 0x73 - ???        */    {0,            0,          0},
/* 0x74 - ???        */    {0,            0,          0},
/* 0x75 - ???        */    {0,            0,          0},
/* 0x76 - ???        */    {0,            0,          0},
/* 0x77 - ???        */    {0,            0,          0},
/* 0x78 - ???        */    {0,            0,          0},
/* 0x78 - ???        */    {0,            0,          0},
/* 0x7A - ???        */    {0,            0,          0},
/* 0x7B - ???        */    {0,            0,          0},
/* 0x7C - ???        */    {0,            0,          0},
/* 0x7D - ???        */    {0,            0,          0},
/* 0x7E - ???        */    {0,            0,          0},
/* 0x7F - ???        */    {0,            0,          0}
};

static uint8_t pause_make_code = {
    0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5
};

static uint8_t print_screen_make_code = {
    0xE0, 0x2A, 0xE0, 0x37
};

static uint8_t print_screen_breake_code = {
    0xE0, 0xB7, 0xE0, 0xAA
};

static void kb_handler(isrp_t *p) {
    static bool_t ctrl_down = False;
    static bool_t shift_down = False;
    static bool_t alt_down = False;
    static bool_t caps_locked = False;
    static bool_t ext_code = False;


    uint8_t scan_code = inportb(KB_DATA_PORT);

    if (scan_code == 0xE1) {
        // omit pause key for now
        kprintf(KPL_PANIC, "\n*** PAUSE! ***\n");
        while (1);
    }

    if (scan_code == EXT_CODE) {
        ext_code = True;
        return;
    }

    int col = 0;

    if (ext_code) {
        col = 2;
        ext_code = False;
    }

    uint16_t key = keymap1[scan_code & 0x7f][col];

    if (scan_code & KB_BREAK_MASK) {
        // bit 7 set => break code
        // if (scan_code == shift_l_break || scan_code == shift_r_break) {
        //     shift_down = False;
        // } else if (scan_code == ctrl_l_break || scan_code == ctrl_r_break) {
        //     ctrl_down = False;
        // } else if (scan_code == alt_l_break || scan_code == alt_r_break) {
        //     alt_down = False;
        // }
        if (key == SHIFT_L || key == SHIFT_R) {
            shift_down = False;
        } else if (key == CTRL_L || key == CTRL_R) {
            ctrl_down = False;
        } else if (key == ALT_L || key == ALT_R) {
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

static void kb_handler1(isrp_t *p) {
    static bool_t ctrl_down = False;
    static bool_t shift_down = False;
    static bool_t alt_down = False;

    static bool_t caps_locked = False;
    static bool_t num_locked = False;
    static bool_t scroll_locked = False;

    static bool_t ext_code = False;


    uint8_t scan_code = inportb(KB_DATA_PORT);

    if (scan_code == 0xE1) {
        // omit pause key for now
        kprintf(KPL_PANIC, "\n*** PAUSE! ***\n");
        while (1);
    }

    if (scan_code == EXT_CODE) {
        ext_code = True;
        return;
    }

    uint16_t key = 0;

    if (ext_code) {
        key = keymap1[scan_code & 0x7f][2];
        ext_code = False;
    } else {
        key = keymap1[scan_code & 0x7f][0];
    }


    if (scan_code & KB_BREAK_MASK) {
        // bit 7 set => break code
        if (key == SHIFT_L || key == SHIFT_R) {
            shift_down = False;
        } else if (key == CTRL_L || key == CTRL_R) {
            ctrl_down = False;
        } else if (key == ALT_L || key == ALT_R) {
            alt_down = False;
        }
        // break of other keys are unimportant
        return;
    } else {
        // make code

        /**
         *  shift ctrl alt caps_lock num_lock scroll_lock are
         *  pure control keys
         */
        if (key == SHIFT_L || key == SHIFT_R) {
            shift_down = True;
        } else if (key == CTRL_L || key == CTRL_R) {
            ctrl_down = True;
        } else if (key == ALT_L || key == ALT_R) {
            alt_down = True;
        } else if (key == CAPS_LOCK) {
            caps_locked = !caps_locked;
        } else if (key == NUM_LOCK) {
            num_locked = !num_locked;
        } else if (key == SCROLL_LOCK) {
            scroll_locked = !scroll_locked;
        } else {
            int col = 0;
            if (key >= 'a' && key <= 'z') {
                // caps_lock only influence letters
                col = (shift_down ^ caps_locked) ? 1 : 0;
            } else {
                col = shift_down ? 1 : 0;
            }
        }
    }
}

void kb_init() {
    register_handler(INT_KB, kb_handler);
}
