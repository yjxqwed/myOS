#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <myos.h>

// virtual address of video memory
#define VIDEO_MEM (__va(0x000B8000))

#define MAXCOL 80
#define MAXROW 25
#define TAB_WIDTH 8


typedef enum COLOR {
    BLACK, BLUE, GREEN, CYAN, RED, PURPLE, BROWN, GRAY,
    DARK_GRAY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, 
    LIGHT_RED, LIGHT_PURPLE, YELLOW, WHITE
} COLOR;


// put a char with foreground color = fg and 
// background color = bg
void scrn_putc(char c, COLOR bg, COLOR fg);
void scrn_puts(const char *str, COLOR bg, COLOR fg);

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

// void init_screen();

void video_mem_enable_paging();
#endif
