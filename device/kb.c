#include <device/kb.h>
#include <arch/x86.h>
#include <common/utils.h>
#include <string.h>
#include <kprintf.h>
#include <sys/isr.h>
#include <common/types.h>
#include <common/debug.h>
#include <thread/sync.h>
#include <device/tty.h>

// keyboard data port
#define KB_DATA_PORT 0x60
// keyboard control port
#define KB_CTRL_PORT 0x64

#define KB_BREAK 0x80
#define KB_SCAN_CODE_MSK 0x7F

#define EXT_CODE 0xE0

static key_code_e keycodes[][2] = {
/* scan_code               {keycode for scan_code, keycode for E0 scan code} */
/* ==========================================================================*/
/* 0x00 - none       */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x01 - ESC        */    {KEYCODE_ESC,             KEYCODE_NONE},
/* 0x02 - '1'        */    {KEYCODE_ALPHA1,          KEYCODE_NONE},
/* 0x03 - '2'        */    {KEYCODE_ALPHA2,          KEYCODE_NONE},
/* 0x04 - '3'        */    {KEYCODE_ALPHA3,          KEYCODE_NONE},
/* 0x05 - '4'        */    {KEYCODE_ALPHA4,          KEYCODE_NONE},
/* 0x06 - '5'        */    {KEYCODE_ALPHA5,          KEYCODE_NONE},
/* 0x07 - '6'        */    {KEYCODE_ALPHA6,          KEYCODE_NONE},
/* 0x08 - '7'        */    {KEYCODE_ALPHA7,          KEYCODE_NONE},
/* 0x09 - '8'        */    {KEYCODE_ALPHA8,          KEYCODE_NONE},
/* 0x0A - '9'        */    {KEYCODE_ALPHA9,          KEYCODE_NONE},
/* 0x0B - '0'        */    {KEYCODE_ALPHA0,          KEYCODE_NONE},
/* 0x0C - '-'        */    {KEYCODE_MINUS,           KEYCODE_NONE},
/* 0x0D - '='        */    {KEYCODE_EQUAL,           KEYCODE_NONE},
/* 0x0E - BS         */    {KEYCODE_BACKSPACE,       KEYCODE_NONE},
/* 0x0F - TAB        */    {KEYCODE_TAB,             KEYCODE_NONE},
/* 0x10 - 'q'        */    {KEYCODE_Q,               KEYCODE_NONE},
/* 0x11 - 'w'        */    {KEYCODE_W,               KEYCODE_NONE},
/* 0x12 - 'e'        */    {KEYCODE_E,               KEYCODE_NONE},
/* 0x13 - 'r'        */    {KEYCODE_R,               KEYCODE_NONE},
/* 0x14 - 't'        */    {KEYCODE_T,               KEYCODE_NONE},
/* 0x15 - 'y'        */    {KEYCODE_Y,               KEYCODE_NONE},
/* 0x16 - 'u'        */    {KEYCODE_U,               KEYCODE_NONE},
/* 0x17 - 'i'        */    {KEYCODE_I,               KEYCODE_NONE},
/* 0x18 - 'o'        */    {KEYCODE_O,               KEYCODE_NONE},
/* 0x19 - 'p'        */    {KEYCODE_P,               KEYCODE_NONE},
/* 0x1A - '['        */    {KEYCODE_LEFTBRACKET,     KEYCODE_NONE},
/* 0x1B - ']'        */    {KEYCODE_RIGHTBRACKET,    KEYCODE_NONE},
/* 0x1C - CR/LF      */    {KEYCODE_ENTER,           KEYCODE_NONE},
/* 0x1D - l. Ctrl    */    {KEYCODE_LCTRL,           KEYCODE_RCTRL},
/* 0x1E - 'a'        */    {KEYCODE_A,               KEYCODE_NONE},
/* 0x1F - 's'        */    {KEYCODE_S,               KEYCODE_NONE},
/* 0x20 - 'd'        */    {KEYCODE_D,               KEYCODE_NONE},
/* 0x21 - 'f'        */    {KEYCODE_F,               KEYCODE_NONE},
/* 0x22 - 'g'        */    {KEYCODE_G,               KEYCODE_NONE},
/* 0x23 - 'h'        */    {KEYCODE_H,               KEYCODE_NONE},
/* 0x24 - 'j'        */    {KEYCODE_J,               KEYCODE_NONE},
/* 0x25 - 'k'        */    {KEYCODE_K,               KEYCODE_NONE},
/* 0x26 - 'l'        */    {KEYCODE_L,               KEYCODE_NONE},
/* 0x27 - ';'        */    {KEYCODE_SEMICOLON,       KEYCODE_NONE},
/* 0x28 - '\''       */    {KEYCODE_QUOTE,           KEYCODE_NONE},
/* 0x29 - '`'        */    {KEYCODE_BACKQUOTE,       KEYCODE_NONE},
/* 0x2A - l. SHIFT   */    {KEYCODE_LSHIFT,          KEYCODE_NONE},
/* 0x2B - '\'        */    {KEYCODE_BACKSLASH,       KEYCODE_NONE},
/* 0x2C - 'z'        */    {KEYCODE_Z,               KEYCODE_NONE},
/* 0x2D - 'x'        */    {KEYCODE_X,               KEYCODE_NONE},
/* 0x2E - 'c'        */    {KEYCODE_C,               KEYCODE_NONE},
/* 0x2F - 'v'        */    {KEYCODE_V,               KEYCODE_NONE},
/* 0x30 - 'b'        */    {KEYCODE_B,               KEYCODE_NONE},
/* 0x31 - 'n'        */    {KEYCODE_N,               KEYCODE_NONE},
/* 0x32 - 'm'        */    {KEYCODE_M,               KEYCODE_NONE},
/* 0x33 - ','        */    {KEYCODE_COMMA,           KEYCODE_NONE},
/* 0x34 - '.'        */    {KEYCODE_PERIOD,          KEYCODE_NONE},
/* 0x35 - '/'        */    {KEYCODE_SLASH,           KEYCODE_NONE},
/* 0x36 - r. SHIFT   */    {KEYCODE_RSHIFT,          KEYCODE_NONE},
/* 0x37 - '*'        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x38 - ALT        */    {KEYCODE_LALT,            KEYCODE_RALT},
/* 0x39 - ' '        */    {KEYCODE_SPACE,           KEYCODE_NONE},
/* 0x3A - CapsLock   */    {KEYCODE_CAPSLOCK,        KEYCODE_NONE},
/* 0x3B - F1         */    {KEYCODE_F1,              KEYCODE_NONE},
/* 0x3C - F2         */    {KEYCODE_F2,              KEYCODE_NONE},
/* 0x3D - F3         */    {KEYCODE_F3,              KEYCODE_NONE},
/* 0x3E - F4         */    {KEYCODE_F4,              KEYCODE_NONE},
/* 0x3F - F5         */    {KEYCODE_F5,              KEYCODE_NONE},
/* 0x40 - F6         */    {KEYCODE_F6,              KEYCODE_NONE},
/* 0x41 - F7         */    {KEYCODE_F7,              KEYCODE_NONE},
/* 0x42 - F8         */    {KEYCODE_F8,              KEYCODE_NONE},
/* 0x43 - F9         */    {KEYCODE_F9,              KEYCODE_NONE},
/* 0x44 - F10        */    {KEYCODE_F10,             KEYCODE_NONE},
/* 0x45 - NumLock    */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x46 - ScrLock    */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x47 - Home       */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x48 - UP(e0)     */    {KEYCODE_NONE,            KEYCODE_UPARROW},
/* 0x49 - PgUp       */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x4A - '-'        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x4B - Left(e0)   */    {KEYCODE_NONE,            KEYCODE_LEFTARROW},
/* 0x4C - MID        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x4D - Right      */    {KEYCODE_NONE,            KEYCODE_RIGHTARROW},
/* 0x4E - '+'        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x4F - End        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x50 - Down(e0)   */    {KEYCODE_NONE,            KEYCODE_DOWNARROW},
/* 0x51 - PgDown     */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x52 - Insert     */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x53 - Delete(e0) */    {KEYCODE_NONE,            KEYCODE_DELETE},
/* 0x54 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x55 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x56 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x57 - F11        */    {KEYCODE_F11,             KEYCODE_NONE},
/* 0x58 - F12        */    {KEYCODE_F12,             KEYCODE_NONE},
/* 0x59 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5A - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5B - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5C - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5D - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5E - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x5F - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x60 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x61 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x62 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x63 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x64 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x65 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x66 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x67 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x68 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x69 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6A - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6B - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6C - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6D - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6E - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x6F - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x70 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x71 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x72 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x73 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x74 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x75 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x76 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x77 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x78 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x78 - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7A - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7B - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7C - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7D - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7E - ???        */    {KEYCODE_NONE,            KEYCODE_NONE},
/* 0x7F - ???        */    {KEYCODE_NONE,            KEYCODE_NONE}
};

static char keycode2char [][2] = {
    [0]                    = {'\0', '\0'},
    [KEYCODE_A]            = {'a', 'A'},
    [KEYCODE_B]            = {'b', 'B'},
    [KEYCODE_C]            = {'c', 'C'},
    [KEYCODE_D]            = {'d', 'D'},
    [KEYCODE_E]            = {'e', 'E'},
    [KEYCODE_F]            = {'f', 'F'},
    [KEYCODE_G]            = {'g', 'G'},
    [KEYCODE_H]            = {'h', 'H'},
    [KEYCODE_I]            = {'i', 'I'},
    [KEYCODE_J]            = {'j', 'J'},
    [KEYCODE_K]            = {'k', 'K'},
    [KEYCODE_L]            = {'l', 'L'},
    [KEYCODE_M]            = {'m', 'M'},
    [KEYCODE_N]            = {'n', 'N'},
    [KEYCODE_O]            = {'o', 'O'},
    [KEYCODE_P]            = {'p', 'P'},
    [KEYCODE_Q]            = {'q', 'Q'},
    [KEYCODE_R]            = {'r', 'R'},
    [KEYCODE_S]            = {'s', 'S'},
    [KEYCODE_T]            = {'t', 'T'},
    [KEYCODE_U]            = {'u', 'U'},
    [KEYCODE_V]            = {'v', 'V'},
    [KEYCODE_W]            = {'w', 'W'},
    [KEYCODE_X]            = {'x', 'X'},
    [KEYCODE_Y]            = {'y', 'Y'},
    [KEYCODE_Z]            = {'z', 'Z'},
    [KEYCODE_ALPHA0]       = {'0', ')'},
    [KEYCODE_ALPHA1]       = {'1', '!'},
    [KEYCODE_ALPHA2]       = {'2', '@'},
    [KEYCODE_ALPHA3]       = {'3', '#'},
    [KEYCODE_ALPHA4]       = {'4', '$'},
    [KEYCODE_ALPHA5]       = {'5', '%'},
    [KEYCODE_ALPHA6]       = {'6', '^'},
    [KEYCODE_ALPHA7]       = {'7', '&'},
    [KEYCODE_ALPHA8]       = {'8', '*'},
    [KEYCODE_ALPHA9]       = {'9', '('},
    [KEYCODE_MINUS]        = {'-', '_'},
    [KEYCODE_EQUAL]        = {'=', '+'},
    [KEYCODE_SPACE]        = {' ', ' '},
    [KEYCODE_BACKQUOTE]    = {'`', '~'},
    [KEYCODE_QUOTE]        = {'\'', '"'},
    [KEYCODE_COMMA]        = {',', '<'},
    [KEYCODE_PERIOD]       = {'.', '>'},
    [KEYCODE_SLASH]        = {'/', '?'},
    [KEYCODE_BACKSLASH]    = {'\\', '|'},
    [KEYCODE_SEMICOLON]    = {';', ':'},
    [KEYCODE_LEFTBRACKET]  = {'[', '{'},
    [KEYCODE_RIGHTBRACKET] = {']', '}'}
};

char get_printable_char(key_info_t ki) {
    key_code_e keycode = __keycode(ki);
    if (keycode == KEYCODE_ENTER) {
        return '\n';
    }
    // if (keycode == KEYCODE_BACKSPACE) {
    //     return '\b';
    // }
    // if (keycode == KEYCODE_TAB) {
    //     return '\t';
    // }
    if (keycode == KEYCODE_NONE || keycode > KEYCODE_RIGHTBRACKET) {
        return '\0';
    }
    bool_t caps = (ki & KIF_CAPS) ? True : False;
    bool_t shift = (ki & KIF_SHIFT) ? True : False;
    if (keycode <= KEYCODE_Z) {
        return keycode2char[keycode][(caps != shift) ? 1 : 0];
    } else {
        return keycode2char[keycode][shift ? 1 : 0];
    }
}

static uint8_t pause_make_code[] = {
    0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5
};

static uint8_t print_screen_make_code[] = {
    0xE0, 0x2A, 0xE0, 0x37
};

static uint8_t print_screen_breake_code[] = {
    0xE0, 0xB7, 0xE0, 0xAA
};

void print_key_info(key_info_t ki) {
    kprintf(
        KPL_DEBUG, "[%s, %s, %s, %s; %d]",
        ki & KIF_CAPS ? "CAPS" : "caps",
        ki & KIF_SHIFT ? "SHIFT" : "shift",
        ki & KIF_ALT ? "ALT" : "alt",
        ki & KIF_CTRL ? "CTRL" : "ctrl",
        __keycode(ki)
    );
    if (ki == (KIF_CTRL | KIF_SHIFT | KEYCODE_C)) {
        kprintf(KPL_NOTICE, "[HELLO WORLD! This myOS by Jiaxing Yang!]");
    }
}

static void *kb_handler(isrp_t *p) {
    static bool_t ctrl_down = False;
    static bool_t shift_down = False;
    static bool_t alt_down = False;
    static bool_t caps_locked = False;

    // if ext_code is True, last scan_code is 0xE0
    static bool_t ext_code = False;

    uint8_t scan_code = inportb(KB_DATA_PORT);
    // unable to handle pause key
    ASSERT(scan_code != 0xE1);

    if (scan_code == EXT_CODE) {
        ext_code = True;
        return NULL;
    }

    key_code_e keycode;
    if (ext_code) {
        keycode = keycodes[scan_code & KB_SCAN_CODE_MSK][1];
        ext_code = False;
    } else {
        keycode = keycodes[scan_code & KB_SCAN_CODE_MSK][0];
    }

    if (keycode == KEYCODE_NONE) {
        return NULL;
    }

    if (scan_code & KB_BREAK) {
        // key released
        switch (keycode) {
            case KEYCODE_LCTRL:
            case KEYCODE_RCTRL:
                ctrl_down = False;
                break;
            case KEYCODE_LSHIFT:
            case KEYCODE_RSHIFT:
                shift_down = False;
                break;
            case KEYCODE_LALT:
            case KEYCODE_RALT:
                alt_down = False;
                break;
            default:
                // ignore other keys' break
                break;
        }
    } else {
        switch (keycode) {
            case KEYCODE_LCTRL:
            case KEYCODE_RCTRL:
                ctrl_down = True;
                break;
            case KEYCODE_LSHIFT:
            case KEYCODE_RSHIFT:
                shift_down = True;
                break;
            case KEYCODE_LALT:
            case KEYCODE_RALT:
                alt_down = True;
                break;
            case KEYCODE_CAPSLOCK:
                caps_locked = !caps_locked;
                break;
            default: {
                key_info_t ki = 0;
                if (ctrl_down) {
                    ki |= KIF_CTRL;
                }
                if (shift_down) {
                    ki |= KIF_SHIFT;
                }
                if (alt_down) {
                    ki |= KIF_ALT;
                }
                if (caps_locked) {
                    ki |= KIF_CAPS;
                }
                ki |= keycode;
                tty_putkey(ki);
                break;
            }
        }
    }

    return NULL;
}

void kb_init() {
    register_handler(INT_KB, kb_handler);
}
