#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <sys/global.h>

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
void putc(char c, COLOR bg, COLOR fg);
// attribute byte
// 7|654|3210
// U|bg | fg
// U: the use of bit7 is unknown


// For more info on text mode cursor 
// https://wiki.osdev.org/Text_Mode_Cursor

void get_cursor(int *row, int *col);

void set_cursor(int row, int col);

void enable_cursor(int cursor_start, int cursor_end);

void disable_cursor();

void init_screen();

void video_mem_enable_paging();
#endif
