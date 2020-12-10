#include <common/types.h>
#include <device/screen.h>
#include <string.h>
#include <arch/x86.h>

static int cursor_row = 0;
static int cursor_col = 0;

static uint32_t video_mem = __pa(VIDEO_MEM);

void video_mem_enable_paging() {
    video_mem = VIDEO_MEM;
}

static void clear_row(int row) {
    char *p = (char *)(video_mem + CHAR_OFFSET(row, 0));
    for (int i = 0; i < ROW_RAM;) {
        p[i++] = BLANK_CHAR;
        p[i++] = BLANK_ATTR;
    }
}

static void clear_screen() {
    char *p = (char *)video_mem;
    for (int i; i < PAGE_RAM; ) {
        p[i++] = BLANK_CHAR;
        p[i++] = BLANK_ATTR;
    }
}

static void scroll(int num_row) {
    char *dest = (char *)(video_mem);
    char *src = (char *)(video_mem + CHAR_OFFSET(num_row, 0));
    uint32_t size = (MAXROW - num_row) * ROW_RAM;
    strncpy(src, dest, size);
    for (int i = 1; i <= num_row; i++) {
        clear_row(MAXROW - i);
    }
}

void get_cursor(int *row, int *col) {
    *row = cursor_row;
    *col = cursor_col;
}

void set_cursor(int row, int col) {
    cursor_row = row;
    cursor_col = col;

    int pos = row * MAXCOL + col;

    outportb(0x3d4, 0x0f);
    outportb(0x3d5, (uint8_t)(pos & 0xff));
    outportb(0x3d4, 0x0e);
    outportb(0x3d5, (uint8_t)((pos >> 8) & 0xff));
}

void putc(char c, COLOR bg, COLOR fg) {
    char *p = (char *)(video_mem + CHAR_OFFSET(cursor_row, cursor_col));
    uint8_t attr = ATTR(bg, fg);
    switch (c) {
        case '\a': {
            break;
        } case '\b': {
            if (cursor_col == 0) {
                if (cursor_row != 0) {
                    cursor_row--;
                    cursor_col = MAXCOL - 1;
                    p = (char *)(video_mem + CHAR_OFFSET(cursor_row, cursor_col));
                    *p++ = BLANK_CHAR;
                    *p++ = attr;
                }
            } else {
                cursor_col--;
                p = (char *)(video_mem + CHAR_OFFSET(cursor_row, cursor_col));
                *p++ = BLANK_CHAR;
                *p++ = attr;
            }
            break;
        } case '\t': {
            break;
        } case '\n': {
            while (cursor_col < MAXCOL) {
                *p++ = BLANK_CHAR;
                *p++ = attr;
                cursor_col++;
            }
            cursor_col = 0;
            cursor_row++;
            break;
        } case '\v': {
            break;
        } case '\f': {
            break;
        } case '\r': {
            cursor_col = 0;
            break;
        } default: {
            *p++ = c;
            *p++ = attr;
            cursor_col++;
            break;
        }
    }
    if (cursor_col >= MAXCOL) {
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= MAXROW) {
        scroll(cursor_row - MAXROW + 1);
        cursor_row = MAXROW - 1;
    }
    set_cursor(cursor_row, cursor_col);
}

void init_screen() {
    clear_screen();
    set_cursor(0, 0);
}