#ifndef __SCREEN_H__
#define __SCREEN_H__

#define COL 80
#define ROW 25

void clear_screen();
void printf(const char* str);

void putch(char c, int x, int y);
void putstr(const char* str);

#endif
