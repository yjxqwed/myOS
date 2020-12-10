#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/**
 *  For the virtual tty (console)
 *  p.s. tty is for TeleTYpewriter
 */

#include <kprintf.h>

// @brief put a string on the console,
// thread safe
void console_puts(const char *str);

void console_init();

#endif