#ifndef __CONSOLE_H__
#define __CONSOLE_H__

// opaque decralation of console struct
typedef struct Console console_t;

#include <myos.h>

typedef enum Color {
    CONS_BLACK, CONS_BLUE, CONS_GREEN, CONS_CYAN,
    CONS_RED, CONS_PURPLE, CONS_BROWN, CONS_GRAY,
    CONS_DARK_GRAY, CONS_LIGHT_BLUE, CONS_LIGHT_GREEN, CONS_LIGHT_CYAN,
    CONS_LIGHT_RED, CONS_LIGHT_PURPLE, CONS_YELLOW, CONS_WHITE
} color_e;


// put a char with foreground color = fg and
// background color = bg
// int console_putc(console_t *cons, char c, color_e bg, color_e fg);

/**
 * @brief puts to console
 * @param count number of characters to put
 */
int console_puts(
    console_t *cons, const char *str, size_t count, color_e bg, color_e fg
);

/**
 * @brief erase the previous char if exists. only for tty_echo use
 */
void console_erase_char(console_t *cons);

// get the tty_no of the current console
int get_curr_console_tty();

void select_console(int console);
console_t *get_console(int tty_no);

/**
 *   a word (16 B) for a char
 *  |   attr    | ascii  |
 *  |7|654|3|210|76543210|
 *  |H|RGB|H|RGB|        |
 *  \____/|\____/
 *    BG  |  FG
 *
 *  H: highlight
 *  RGB: red green blue
 *  BG: background
 *  FG: foreground
 */


// For more info on text mode cursor:
// https://wiki.osdev.org/Text_Mode_Cursor

void console_get_cursor(console_t *cons, int *row, int *col);

void console_set_cursor(console_t *cons, int row, int col);


/**
 * @brief set the start row of the display
 * @param row row offset
 */
void set_video_start_row(uint32_t row);

#endif
