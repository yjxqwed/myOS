#include <common/types.h>
#include <common/debug.h>

#include <lib/string.h>

#include <arch/x86.h>

#include <thread/sync.h>

#include <device/tty.h>
#include <device/console.h>


// offset of the char at (row, col)
#define CHAR_OFFSET(row, col) ((row) * MAXCOL + (col))


/**
 * @brief assemble a char to display
 * @param ch the char to display
 * @param bg background color
 * @param fg foreground color
 */
#define __disp_char(ch, bg, fg) (uint16_t)(((bg) << 12) | ((fg) << 8) | (ch))

#define BLANK_CHAR __disp_char(' ', BLACK, GRAY)


struct Console {
    uint16_t *video_mem;
    // cursor position
    uint16_t cursor_row_base;
    uint16_t cursor_row;
    uint16_t cursor_col;
    // record the col for write
    uint16_t write_cursor_col;
    // output mutex of this console
    mutex_t lock;
    int tty_no;
};

static console_t consoles[NR_TTY];
static int current_console = -1;

console_t *init_console(int tty_no) {
    ASSERT(tty_no >= 0 && tty_no < NR_TTY);
    console_t *cons = &(consoles[tty_no]);
    cons->video_mem = (uint16_t *)(VIDEO_MEM + tty_no * PAGE_RAM);
    cons->cursor_col = 0;
    cons->cursor_row = 0;
    cons->cursor_row_base = tty_no * MAXROW;
    mutex_init(&(cons->lock));
    cons->tty_no = tty_no;
    return cons;
}

static inline void __clear_row(console_t *cons, int row) {
    for (int i = 0; i < MAXCOL; i++) {
        cons->video_mem[CHAR_OFFSET(row, i)] = BLANK_CHAR;
    }
}

static void __set_cursor(console_t *cons) {
    if (current_console == -1 || (cons != &(consoles[current_console]))) {
        return;
    }
    int pos = (
        cons->cursor_row_base + cons->cursor_row
    ) * MAXCOL + cons->cursor_col;
    outportb(0x3d4, 0x0f);
    outportb(0x3d5, (uint8_t)(pos & 0xff));
    outportb(0x3d4, 0x0e);
    outportb(0x3d5, (uint8_t)((pos >> 8) & 0xff));
}

void console_set_cursor(console_t *cons, int row, int col) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->lock));
    cons->cursor_row = row;
    cons->cursor_col = col;
    __set_cursor(cons);
    mutex_unlock(&(cons->lock));
}

static void __get_cursor(console_t *cons, int *row, int *col) {
    *row = cons->cursor_row;
    *col = cons->cursor_col;
}

void console_get_cursor(console_t *cons, int *row, int *col) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->lock));
    __get_cursor(cons, row, col);
    mutex_unlock(&(cons->lock));
}

void clear_screen(console_t *cons) {
    ASSERT(cons != NULL);
    mutex_lock(&(cons->lock));
    for (int i = 0; i < MAXROW; i++) {
        __clear_row(cons, i);
    }
    cons->cursor_col = 0;
    cons->cursor_row = 0;
    __set_cursor(cons);
    mutex_unlock(&(cons->lock));
}

void clear_curr_console() {
    int console_no = get_current_thread()->tty_no;
    if (console_no < 0 || console_no >= NR_TTY) {
        return;
    }
    clear_screen(consoles + console_no);
}

// scroll up by one line
static void __scrollup(console_t *cons) {
    uint16_t *dest = cons->video_mem;
    uint16_t *src = cons->video_mem + MAXCOL;
    memcpy(src, dest, ROW_RAM * (MAXROW - 1));
    __clear_row(cons, MAXROW - 1);
}

void console_erase_char(console_t *cons) {
    ASSERT(cons != NULL);
    ASSERT(get_int_status() == INTERRUPT_OFF);
    if (cons->cursor_col > cons->write_cursor_col) {
        cons->cursor_col--;
        uint16_t *p = &(cons->video_mem[
            CHAR_OFFSET(cons->cursor_row, cons->cursor_col)
        ]);
        *p = __disp_char(' ', BLACK, GRAY);
        __set_cursor(cons);
    }
}

static void __putc(console_t *cons, char c, COLOR bg, COLOR fg) {
    if (c == '\0') {
        return;
    }
    uint16_t *p = &(cons->video_mem[
        CHAR_OFFSET(cons->cursor_row, cons->cursor_col)
    ]);
    switch (c) {
        case '\a': {
            break;
        } case '\b': {
            // \b is really special, should only treat it as a control character
            // Don't output \b
            break;
        } case '\t': {
            // omit Tab for now
            break;
        } case '\n': {
            while (cons->cursor_col < MAXCOL) {
                *p = __disp_char(' ', bg, fg);
                p++;
                cons->cursor_col++;
            }
            cons->cursor_col = 0;
            cons->write_cursor_col = 0;
            (cons->cursor_row)++;
            break;
        } case '\v': {
            break;
        } case '\f': {
            break;
        } case '\r': {
            break;
        } default: {
            *p = __disp_char(c, bg, fg);
            (cons->cursor_col)++;
            break;
        }
    }
    if (cons->cursor_col >= MAXCOL) {
        cons->cursor_col = 0;
        cons->write_cursor_col = 0;
        (cons->cursor_row)++;
    }
    if (cons->cursor_row >= MAXROW) {
        cons->cursor_row = MAXROW - 1;
        __scrollup(cons);
    }
}

static int __puts(
    console_t *cons, const char *str, size_t count,
    COLOR bg, COLOR fg, bool_t set_write_out_col
) {
    ASSERT(cons != NULL);
    if (cons == &(consoles[0])) {
        return 0;
    }
    for (int i = 0; i < count; i++) {
        __putc(cons, str[i], bg, fg);
    }
    __set_cursor(cons);
    if (set_write_out_col) {
        cons->write_cursor_col = cons->cursor_col;
    }
    return count;
}

int console_puts_nolock(
    console_t *cons, const char *str, size_t count, COLOR bg, COLOR fg,
    bool_t set_write_out_col
) {
    INT_STATUS old_status = disable_int();
    int ret = __puts(cons, str, count, bg, fg, set_write_out_col);
    set_int_status(old_status);
    return ret;
}

int console_puts(
    console_t *cons, const char *str, size_t count, COLOR bg, COLOR fg,
    bool_t set_write_out_col
) {
    mutex_lock(&(cons->lock));
    int ret = __puts(cons, str, count, bg, fg, set_write_out_col);
    mutex_unlock(&(cons->lock));
    return ret;
}

int get_curr_console_tty() {
    return current_console;
}

void set_video_start_row(uint32_t row) {
    uintptr_t video_mem = __pa(VIDEO_MEM + row * MAXCOL);
    outportb(0x3d4, 0x0d);
    outportb(0x3d5, video_mem);
    outportb(0x3d4, 0x0c);
    outportb(0x3d5, video_mem >> 8);
}

void select_console(int console) {
    ASSERT(0 <= console && console < NR_TTY);
    if (current_console == console) {
        return;
    }
    current_console = console;
    console_t *cons = &(consoles[console]);
    ASSERT(get_int_status() == INTERRUPT_OFF);
    set_video_start_row(cons->cursor_row_base);
    __set_cursor(cons);
}
