#ifndef __CONSOLE_H__
#define __CONSOLE_H__

// opaque decralation of console struct
typedef struct Console console_t;

#include <myos.h>
#include <device/screen.h>

/**
 * @brief puts to console
 * @param count number of characters to put
 * @param set_write_out_col for console_erase_char not to erase the output chars
 */
int console_puts(
    console_t *cons, const char *str, size_t count, COLOR bg, COLOR fg,
    bool_t set_write_out_col
);

// same as console puts but no lock, only used by kernel to print something.
int console_puts_nolock(
    console_t *cons, const char *str, size_t count, COLOR bg, COLOR fg,
    bool_t set_write_out_col
);

/**
 * @brief erase the previous char if exists. only for tty_echo use
 */
void console_erase_char(console_t *cons);

// get the tty_no of the current console
int get_curr_console_tty();

void select_console(int console);
console_t *init_console(int tty_no);


// For more info on text mode cursor:
// https://wiki.osdev.org/Text_Mode_Cursor

void console_get_cursor(console_t *cons, int *row, int *col);

void console_set_cursor(console_t *cons, int row, int col);


/**
 * @brief set the start row of the display
 * @param row row offset
 */
void set_video_start_row(uint32_t row);

/**
 * @brief clear the console
 */
void clear_screen(console_t *cons);
void clear_curr_console();
#endif
