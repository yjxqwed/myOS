#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <myos.h>

// virtual address of video memory
#define VIDEO_MEM (__va(0x000B8000))

#define MAXCOL 80
#define MAXROW 25
#define TAB_WIDTH 8


// offset of the char at (row, col)
#define CHAR_OFFSET(row, col) ((row * MAXCOL + col) * 2)

// size in bytes of a whole row
#define ROW_RAM (MAXCOL * 2)
// size in bytes of a whole page (screen)
#define PAGE_RAM (MAXROW * ROW_RAM)

#define ATTR(bg, fg) (uint8_t)((bg << 4) | fg)


typedef enum COLOR {
    BLACK, BLUE, GREEN, CYAN, RED, PURPLE, BROWN, GRAY,
    DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, 
    LIGHT_RED, LIGHT_PURPLE, YELLOW, WHITE
} COLOR;

#define BLANK_CHAR (' ')
#define BLANK_ATTR ATTR(BLACK, GRAY)

// void clear_screen();
// void printf(const char* str);

// void putch(char c, int x, int y);
// void putstr(const char* str);

// put a char with foreground color = fg and 
// background color = bg
void scrn_putc(char c, COLOR bg, COLOR fg);
void scrn_puts(const char *str, COLOR bg, COLOR fg);
void scrn_putc_safe(char c, COLOR bg, COLOR fg);
void scrn_puts_safe(const char *str, COLOR bg, COLOR fg);


/**
 *   a word (16 B) for a char
 *  |   attr    | ascii  |
 *  |7|654|3|210|76543210|
 *  |F|RGB|H|RGB|        |
 *  \____/|\____/
 *    BG  |  FG
 *
 *  F: flash
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
