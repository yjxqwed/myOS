#ifndef __KB_H__
#define __KB_H__

// for keyboard

void kb_init();

/**
 *  |   high byte   |low byte|
 *  |7|6|5|4|3|2|1|0|76543210|
 *  |       |C|A|S|E|        |
 *  |   U   |T|L|H|X|  char  |
 *  |       |R|T|F|T|        |
 * 
 *  U: unused
 *  CTR: control
 *  ALT: alt
 *  SHF: shift
 *  EXT: extended (with e0)
 */

#define KB_CTRL_MASK    0x0800
#define KB_ALT_MASK     0x0400
#define KB_SHIFT_MASK   0x0200
#define KB_EXT_MASK     0x0100
#define KB_BREAK_MASK   0x0080
#define KB_KEYCODE_MASK 0x007f

#define __with_ctrl(key)  ((key) & KB_CTRL_MASK)
#define __with_alt(key)   ((key) & KB_ALT_MASK)
#define __with_shift(key) ((key) & KB_SHIFT_MASK)
#define __is_break(key)   ((key) & KB_BREAK_MASK)
#define __keycode(key)    ((key) & KB_KEYCODE_MASK)

/* Special keys */
#define ESC          (0x01 + KB_EXT_MASK)    /* Esc */
#define TAB          (0x02 + KB_EXT_MASK)    /* Tab */
#define ENTER        (0x03 + KB_EXT_MASK)    /* Enter */
#define BACKSPACE    (0x04 + KB_EXT_MASK)    /* BackSpace */

#define GUI_L        (0x05 + KB_EXT_MASK)    /* L GUI */
#define GUI_R        (0x06 + KB_EXT_MASK)    /* R GUI */
#define APPS         (0x07 + KB_EXT_MASK)    /* APPS */

/* Shift, Ctrl, Alt */
#define SHIFT_L       (0x08 + KB_EXT_MASK)    /* L Shift */
#define SHIFT_R       (0x09 + KB_EXT_MASK)    /* R Shift */
#define CTRL_L        (0x0A + KB_EXT_MASK)    /* L Ctrl */
#define CTRL_R        (0x0B + KB_EXT_MASK)    /* R Ctrl */
#define ALT_L         (0x0C + KB_EXT_MASK)    /* L Alt */
#define ALT_R         (0x0D + KB_EXT_MASK)    /* R Alt */

/* Lock keys */
#define CAPS_LOCK     (0x0E + KB_EXT_MASK)    /* Caps Lock */
#define NUM_LOCK      (0x0F + KB_EXT_MASK)    /* Number Lock */
#define SCROLL_LOCK   (0x10 + KB_EXT_MASK)    /* Scroll Lock */

/* Function keys */
#define F1            (0x11 + KB_EXT_MASK)    /* F1 */
#define F2            (0x12 + KB_EXT_MASK)    /* F2 */
#define F3            (0x13 + KB_EXT_MASK)    /* F3 */
#define F4            (0x14 + KB_EXT_MASK)    /* F4 */
#define F5            (0x15 + KB_EXT_MASK)    /* F5 */
#define F6            (0x16 + KB_EXT_MASK)    /* F6 */
#define F7            (0x17 + KB_EXT_MASK)    /* F7 */
#define F8            (0x18 + KB_EXT_MASK)    /* F8 */
#define F9            (0x19 + KB_EXT_MASK)    /* F9 */
#define F10           (0x1A + KB_EXT_MASK)    /* F10 */
#define F11           (0x1B + KB_EXT_MASK)    /* F11 */
#define F12           (0x1C + KB_EXT_MASK)    /* F12 */

/* Control Pad */
#define PRINTSCREEN   (0x1D + KB_EXT_MASK)    /* Print Screen */
#define PAUSEBREAK    (0x1E + KB_EXT_MASK)    /* Pause/Break */
#define INSERT        (0x1F + KB_EXT_MASK)    /* Insert */
#define DELETE        (0x20 + KB_EXT_MASK)    /* Delete */
#define HOME          (0x21 + KB_EXT_MASK)    /* Home */
#define END           (0x22 + KB_EXT_MASK)    /* End */
#define PAGEUP        (0x23 + KB_EXT_MASK)    /* Page Up */
#define PAGEDOWN      (0x24 + KB_EXT_MASK)    /* Page Down */
#define UP            (0x25 + KB_EXT_MASK)    /* Up */
#define DOWN          (0x26 + KB_EXT_MASK)    /* Down */
#define LEFT          (0x27 + KB_EXT_MASK)    /* Left */
#define RIGHT         (0x28 + KB_EXT_MASK)    /* Right */

/* ACPI keys */
#define POWER         (0x29 + KB_EXT_MASK)    /* Power */
#define SLEEP         (0x2A + KB_EXT_MASK)    /* Sleep */
#define WAKE          (0x2B + KB_EXT_MASK)    /* Wake Up */

/* Num Pad */
#define PAD_SLASH     (0x2C + KB_EXT_MASK)    /* / */
#define PAD_STAR      (0x2D + KB_EXT_MASK)    /* * */
#define PAD_MINUS     (0x2E + KB_EXT_MASK)    /* - */
#define PAD_PLUS      (0x2F + KB_EXT_MASK)    /* + */
#define PAD_ENTER     (0x30 + KB_EXT_MASK)    /* Enter */
#define PAD_DOT       (0x31 + KB_EXT_MASK)    /* . */
#define PAD_0         (0x32 + KB_EXT_MASK)    /* 0 */
#define PAD_1         (0x33 + KB_EXT_MASK)    /* 1 */
#define PAD_2         (0x34 + KB_EXT_MASK)    /* 2 */
#define PAD_3         (0x35 + KB_EXT_MASK)    /* 3 */
#define PAD_4         (0x36 + KB_EXT_MASK)    /* 4 */
#define PAD_5         (0x37 + KB_EXT_MASK)    /* 5 */
#define PAD_6         (0x38 + KB_EXT_MASK)    /* 6 */
#define PAD_7         (0x39 + KB_EXT_MASK)    /* 7 */
#define PAD_8         (0x3A + KB_EXT_MASK)    /* 8 */
#define PAD_9         (0x3B + KB_EXT_MASK)    /* 9 */
#define PAD_UP        PAD_8                /* Up */
#define PAD_DOWN      PAD_2                /* Down */
#define PAD_LEFT      PAD_4                /* Left */
#define PAD_RIGHT     PAD_6                /* Right */
#define PAD_HOME      PAD_7                /* Home */
#define PAD_END       PAD_1                /* End */
#define PAD_PAGEUP    PAD_9                /* Page Up */
#define PAD_PAGEDOWN  PAD_3                /* Page Down */
#define PAD_INS       PAD_0                /* Ins */
#define PAD_MID       PAD_5                /* Middle key */
#define PAD_DEL       PAD_DOT              /* Del */


#endif
