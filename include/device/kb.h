#ifndef __KB_H__
#define __KB_H__

// for keyboard

#include <common/types.h>

void kb_init();

typedef enum KeyCode {
    // unknown key
    KEYCODE_NONE = 0,
    // letter keys
    KEYCODE_A,
    KEYCODE_B,
    KEYCODE_C,
    KEYCODE_D,
    KEYCODE_E,
    KEYCODE_F,
    KEYCODE_G,
    KEYCODE_H,
    KEYCODE_I,
    KEYCODE_J,
    KEYCODE_K,
    KEYCODE_L,
    KEYCODE_M,
    KEYCODE_N,
    KEYCODE_O,
    KEYCODE_P,
    KEYCODE_Q,
    KEYCODE_R,
    KEYCODE_S,
    KEYCODE_T,
    KEYCODE_U,
    KEYCODE_V,
    KEYCODE_W,
    KEYCODE_X,
    KEYCODE_Y,
    KEYCODE_Z,
    // keys on the top of the alphanumeric keyboard.
    KEYCODE_ALPHA0,  // 0 )
    KEYCODE_ALPHA1,  // 1 !
    KEYCODE_ALPHA2,  // 2 @
    KEYCODE_ALPHA3,  // 3 #
    KEYCODE_ALPHA4,  // 4 $
    KEYCODE_ALPHA5,  // 5 %
    KEYCODE_ALPHA6,  // 6 ^
    KEYCODE_ALPHA7,  // 7 &
    KEYCODE_ALPHA8,  // 8 *
    KEYCODE_ALPHA9,  // 9 (
    KEYCODE_MINUS,   // - _
    KEYCODE_EQUAL,   // = +
    // other printable keys
    KEYCODE_SPACE,
    KEYCODE_BACKQUOTE,     // ` ~
    KEYCODE_QUOTE,         // ' ""
    KEYCODE_COMMA,         // , <
    KEYCODE_PERIOD,        // . >
    KEYCODE_SLASH,         // / ?
    KEYCODE_BACKSLASH,     // \ |
    KEYCODE_SEMICOLON,     // ; :
    KEYCODE_LEFTBRACKET,   // [ {
    KEYCODE_RIGHTBRACKET,  // ] }
    // special keys
    KEYCODE_ESC,
    KEYCODE_DELETE,
    KEYCODE_BACKSPACE,
    KEYCODE_CAPSLOCK,
    KEYCODE_LSHIFT,
    KEYCODE_LCTRL,
    KEYCODE_LALT,
    KEYCODE_RSHIFT,
    KEYCODE_RCTRL,
    KEYCODE_RALT,
    KEYCODE_UPARROW,
    KEYCODE_LEFTARROW,
    KEYCODE_DOWNARROW,
    KEYCODE_RIGHTARROW,
    KEYCODE_ENTER,
    KEYCODE_TAB,
    // functional keys
    KEYCODE_F1,
    KEYCODE_F2,
    KEYCODE_F3,
    KEYCODE_F4,
    KEYCODE_F5,
    KEYCODE_F6,
    KEYCODE_F7,
    KEYCODE_F8,
    KEYCODE_F9,
    KEYCODE_F10,
    KEYCODE_F11,
    KEYCODE_F12,
} key_code_e;

/**
 *  |  flags  | keycode |
 *  +---------+---------+
 *  |31     16|15      0|
 */
typedef uint32_t key_info_t;

// Key Info Flags
#define KIF_CTRL  (1 << 16)
#define KIF_ALT   (1 << 17)
#define KIF_SHIFT (1 << 18)
#define KIF_CAPS  (1 << 19)

#define __keycode(key_info)  ((key_info) & 0xffff)
#define __keyflags(key_info) ((key_info) & 0xffff0000)

// get a key stroke
// key_info_t getkey();

/**
 * @brief get the printable char
 * @param ki keyinfo
 */
char get_printable_char(key_info_t ki);


// for debug only
void print_key_info(key_info_t ki);
#endif
