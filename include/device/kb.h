#ifndef __KB_H__
#define __KB_H__

// for keyboard

void kb_init();

/**
 *  |   high byte   |low byte |
 *  |7|6|5|4|3|2|1|0|7|6543210|
 *  |       |C|A|S|E|B|KEYCODE|
 *  |   U   |T|L|H|X|R|       |
 *  |       |R|T|F|T|K|       |
 * 
 *  U: unused
 *  CTR: control
 *  ALT: alt
 *  SHF: shift
 *  EXT: extended (with e0)
 *  BRK: break code (bit 7 set)
 */

#define CTRL_MASK    0x0800
#define ALT_MASK     0x0400
#define SHIFT_MASK   0x0200
#define EXT_MASK     0x0100
#define BRK_MASK     0x0080
#define KEYCODE_MASK 0x007f

#define __with_ctrl(key)  ((key) & CTRL_MASK)
#define __with_alt(key)   ((key) & ALT_MASK)
#define __with_shift(key) ((key) & SHIFT_MASK)
#define __is_break(key)   ((key) & BRK_MASK)
#define __keycode(key)    ((key) & KEYCODE_MASK)


#endif
