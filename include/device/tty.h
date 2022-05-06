#ifndef __TTY_H__
#define __TTY_H__

// opaque declaration of struct TTY
// TTY is for TeleTYpewriter
typedef struct TTY tty_t;

#include <device/kb.h>
#include <device/console.h>

// we have 6 TTYs in total
#define NR_TTY 6

// For keyboard interrupt handler to use only.
// Put key into current the tty's buffer.
// this tty is the one whose console is active.
void tty_putkey(key_info_t ki);

void tty_flush_key_buffer(int tty_no);

// get a keyinfo from tty
key_info_t tty_getkey(int tty_no);
// get a keyinfo from tty (the tty of the process which needs the keyinfo)
key_info_t tty_getkey_curr();

/**
 * @brief print str to the console; this function
 *        should be called by sys_write only
 */
int tty_puts(
    int tty_no, const char *str, size_t count,
    color_e bg, color_e fg
);

/**
 * @brief print str to the current tty;
 *        this function should be used by console_kprintf only
 */
int tty_puts_curr(
    const char *str, size_t count,
    color_e bg, color_e fg
);


void tty_init();

#endif
