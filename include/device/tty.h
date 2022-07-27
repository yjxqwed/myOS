#ifndef __TTY_H__
#define __TTY_H__

// TTY is for TeleTYpewriter

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

// read up to count chars from tty <tty_no>
int tty_read(int tty_no, char *buf, size_t count);

// read up to count chars from current tty
int tty_read_curr(char *buf, size_t count);

/**
 * @brief print str to the console; this function
 *        should be called by sys_write only
 */
int tty_puts(int tty_no, const char *str, size_t count, COLOR bg, COLOR fg);

/**
 * @brief print str to the current tty;
 *        this function should be used by console_kprintf only
 */
int tty_puts_curr(const char *str, size_t count, COLOR bg, COLOR fg);

int tty_puts_task(const char *str, size_t count, COLOR bg, COLOR fg);

void tty_init();

#endif
