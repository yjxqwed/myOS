#ifndef __CONSOLE_H__
#define __CONSOLE_H__

// opaque decralation of console struct
typedef struct Console console_t;

#include <myos.h>

// there are 3 virtual consoles
#define NR_VCONSOLES 3

typedef enum Color {
    BLACK, BLUE, GREEN, CYAN, RED, PURPLE, BROWN, GRAY,
    DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, 
    LIGHT_RED, LIGHT_PURPLE, YELLOW, WHITE
} color_e;


// put a char with foreground color = fg and 
// background color = bg
void scrn_putc(char c, color_e bg, color_e fg);
void scrn_puts(const char *str, color_e bg, color_e fg);
void scrn_putc_safe(char c, color_e bg, color_e fg);
void scrn_puts_safe(const char *str, color_e bg, color_e fg);


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


// For more info on text mode cursor 
// https://wiki.osdev.org/Text_Mode_Cursor

void get_cursor(int *row, int *col);

void set_cursor(int row, int col);

void enable_cursor(int cursor_start, int cursor_end);

void disable_cursor();

void init_screen();

void video_mem_enable_paging();
#endif
