#include <common/types.h>
#include <device/console.h>
#include <string.h>
#include <arch/x86.h>
#include <thread/sync.h>
#include <common/debug.h>
#include <device/tty.h>
// #include <kprintf.h>

// virtual address of video memory
#define CONSOLE_VIDEO_MEM (__va(0xB8000))

#define CONSOLE_MAXCOL 80
#define CONSOLE_MAXROW 25
#define CONSOLE_TAB_WIDTH 8

typedef uint16_t disp_char_t;

// offset of the char at (row, col)
#define CONSOLE_CHAR_OFFSET(row, col) ((row) * CONSOLE_MAXCOL + (col))

// size in bytes of a whole row
#define CONSOLE_ROW_RAM (CONSOLE_MAXCOL * 2)
// size in bytes of a whole page (screen)
#define CONSOLE_PAGE_RAM (CONSOLE_MAXROW * CONSOLE_ROW_RAM)


/**
 * @brief assemble a char to display
 * @param ch the char to display
 * @param bg background color
 * @param fg foreground color
 */
#define __disp_char(ch, bg, fg) \
    (uint16_t)(((bg) << 12) | ((fg) << 8) | (ch))

#define CONSOLE_BLANK_CHAR __disp_char(' ', CONS_BLACK, CONS_GRAY)


struct Console {
    uint16_t *video_mem;
    // cursor position
    uint16_t cursor_row_base;
    uint16_t cursor_row;
    uint16_t cursor_col;
    // output mutex of this console
    mutex_t cons_mutex;
    int tty_no;
};

static console_t consoles[NR_TTY];
static int current_console = -1;

console_t *get_console(int tty_no) {
    ASSERT(tty_no >= 0 && tty_no < NR_TTY);
    console_t *cons = &(consoles[tty_no]);
    cons->video_mem = (uint16_t *)(CONSOLE_VIDEO_MEM + tty_no * CONSOLE_PAGE_RAM);
    cons->cursor_col = 0;
    cons->cursor_row = 0;
    cons->cursor_row_base = tty_no * CONSOLE_MAXROW;
    mutex_init(&(cons->cons_mutex));
    cons->tty_no = tty_no;
    return cons;
}

static inline void clear_row(console_t *cons, int row) {
    for (int i = 0; i < CONSOLE_MAXCOL; i++) {
        cons->video_mem[CONSOLE_CHAR_OFFSET(row, i)] = CONSOLE_BLANK_CHAR;
    }
}

static inline void clear_screen(console_t *cons) {
    for (int i = 0; i < CONSOLE_MAXROW; i++) {
        clear_row(cons, i);
    }
}

// static void scroll(console_t *cons, int num_row) {
//     char *dest = (char *)(video_mem);
//     char *src = (char *)(video_mem + CONSOLE_CHAR_OFFSET(num_row, 0));
//     uint32_t size = (CONSOLE_MAXROW - num_row) * CONSOLE_ROW_RAM;
//     strncpy(src, dest, size);
//     for (int i = 1; i <= num_row; i++) {
//         clear_row(CONSOLE_MAXROW - i);
//     }
// }

// scroll up by one line
static void scrollup(console_t *cons) {
    uint16_t *dest = cons->video_mem;
    uint16_t *src = cons->video_mem + CONSOLE_MAXCOL;
    memcpy(src, dest, CONSOLE_ROW_RAM * (CONSOLE_MAXROW - 1));
    clear_row(cons, CONSOLE_MAXROW - 1);
}

static void __get_cursor(console_t *cons, int *row, int *col) {
    *row = cons->cursor_row;
    *col = cons->cursor_col;
}

void console_get_cursor(console_t *cons, int *row, int *col) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->cons_mutex));
    __get_cursor(cons, row, col);
    mutex_unlock(&(cons->cons_mutex));
}

static void __set_cursor(console_t *cons) {
    if (current_console == -1 || (cons != &(consoles[current_console]))) {
        return;
    }
    int pos = (
        cons->cursor_row_base + cons->cursor_row
    ) * CONSOLE_MAXCOL + cons->cursor_col;
    outportb(0x3d4, 0x0f);
    outportb(0x3d5, (uint8_t)(pos & 0xff));
    outportb(0x3d4, 0x0e);
    outportb(0x3d5, (uint8_t)((pos >> 8) & 0xff));
}

void console_set_cursor(console_t *cons, int row, int col) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->cons_mutex));
    cons->cursor_row = row;
    cons->cursor_col = col;
    __set_cursor(cons);
    mutex_unlock(&(cons->cons_mutex));
}

static void __putc(console_t *cons, char c, color_e bg, color_e fg) {
    if (c == '\0') {
        return;
    }
    uint16_t *p = &(cons->video_mem[
        CONSOLE_CHAR_OFFSET(cons->cursor_row, cons->cursor_col)
    ]);
    switch (c) {
        case '\a': {
            break;
        } case '\b': {
            if (cons->cursor_col == 0) {
                if (cons->cursor_row != 0) {
                    cons->cursor_row--;
                    cons->cursor_col = CONSOLE_MAXCOL - 1;
                    p = cons->video_mem[
                        CONSOLE_CHAR_OFFSET(cons->cursor_row, cons->cursor_col)
                    ];
                    *p = __disp_char(' ', bg, fg);
                }
            } else {
                cons->cursor_col--;
                p = cons->video_mem[
                    CONSOLE_CHAR_OFFSET(cons->cursor_row, cons->cursor_col)
                ];
                *p = __disp_char(' ', bg, fg);
            }
            break;
        } case '\t': {
            break;
        } case '\n': {
            while (cons->cursor_col < CONSOLE_MAXCOL) {
                *p = __disp_char(' ', bg, fg);
                p++;
                cons->cursor_col++;
            }
            cons->cursor_col = 0;
            (cons->cursor_row)++;
            break;
        } case '\v': {
            break;
        } case '\f': {
            break;
        } case '\r': {
            break;
        } default: {
            // MAGICBP;
            *p = __disp_char(c, bg, fg);
            (cons->cursor_col)++;
            break;
        }
    }
    if (cons->cursor_col >= CONSOLE_MAXCOL) {
        cons->cursor_col = 0;
        (cons->cursor_row)++;
    }
    if (cons->cursor_row >= CONSOLE_MAXROW) {
        // scroll(cursor_row - CONSOLE_MAXROW + 1);
        cons->cursor_row = CONSOLE_MAXROW - 1;
        scrollup(cons);
    }
}

void console_putc(console_t *cons, char c, color_e bg, color_e fg) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->cons_mutex));
    __putc(cons, c, bg, fg);
    __set_cursor(cons);
    mutex_unlock(&(cons->cons_mutex));
}

void console_puts(console_t *cons, const char *str, color_e bg, color_e fg) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->cons_mutex));
    for (int i = 0; str[i] != '\0'; i++) {
        __putc(cons, str[i], bg, fg);
    }
    __set_cursor(cons);
    mutex_unlock(&(cons->cons_mutex));
}

int get_curr_console_tty() {
    return current_console;
}

void set_video_start_row(uint32_t row) {
    uintptr_t video_mem = __pa(CONSOLE_VIDEO_MEM + row * CONSOLE_MAXCOL);
    outportb(0x3d4, 0x0d);
    outportb(0x3d5, video_mem);
    outportb(0x3d4, 0x0c);
    outportb(0x3d5, video_mem >> 8);
}

void select_console(int console) {
    if (current_console == console) {
        return;
    }
    current_console = console;
    console_t *cons = &(consoles[console]);
    ASSERT(get_int_status() == INTERRUPT_OFF);
    set_video_start_row(cons->cursor_row_base);
    __set_cursor(cons);
}
