#ifndef __KB_H__
#define __KB_H__

// for keyboard

#include <common/types.h>

void kb_init();

typedef enum KeyCode {
    KEYCODE_NONE = 0,   // not a key

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
    KEYCODE_BACKQUOTE,     // ` ~
    KEYCODE_QUOTE,         // ' ""
    KEYCODE_SPACE,
    KEYCODE_COMMA,         // , <
    KEYCODE_PERIOD,        // . >
    KEYCODE_SLASH,         // / ?
    KEYCODE_BACKSLASH,     // \ |
    KEYCODE_SEMICOLON,     // ; :
    KEYCODE_LEFTBRACKET,   // [ {
    KEYCODE_RIGHTBRACKET,  // ] }

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

// typedef struct KeyInfo {
//     uint32_t key_flags;
//     key_code_e keycode;
// } key_info_t;

/**
 *  |  flags  |keycode|
 *  +---------+-------+
 *  |31      8|7     0|
 */
typedef uint32_t key_info_t;

#define KIF_CTRL  (1 << 8)
#define KIF_ALT   (1 << 9)
#define KIF_SHIFT (1 << 10)
#define KIF_CAPS  (1 << 11)

#define __keycode(key_info)  ((key_info) & 0xff)
#define __keyflags(key_info) ((key_info) & 0xffffff00)

// get a key stroke
key_info_t getkey();


// for debug only
void print_key_info(key_info_t ki);
#endif
