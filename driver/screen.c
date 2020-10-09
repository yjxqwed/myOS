// #include "types.h"
// #include "screen.h"
// #include "string.h"

#include <common/types.h>
#include <driver/screen.h>
#include <string.h>
#include <sys/system.h>  // for inb outb

static int cursor_row = 0;
static int cursor_col = 0;

// void clear_screen() {
//     volatile int8_t* video_buffer = (volatile int8_t*)0xB8000;
//     const unsigned int total = COL * ROW * 2;
//     int i;
//     while(i < total)
//     {
//         video_buffer[i++] = ' ';
//         video_buffer[i++] = 0x0f;              
//     }
// }


static void clear_row(int row) {
    char *p = (char *)(VIDEO_MEM + CHAR_OFFSET(row, 0));
    for (int i = 0; i < ROW_RAM;) {
        p[i++] = BLANK_CHAR;
        p[i++] = BLANK_ATTR;
    }
}

static void clear_screen() {
    char *p = (char *)VIDEO_MEM;
    for (int i; i < PAGE_RAM; ) {
        p[i++] = BLANK_CHAR;
        p[i++] = BLANK_ATTR;
    }
}

static void scroll(int num_row) {
    char *dest = (char *)(VIDEO_MEM);
    char *src = (char *)(VIDEO_MEM + CHAR_OFFSET(num_row, 0));
    uint32_t size = (MAXROW - num_row) * ROW_RAM;
    strncpy(src, dest, size);
    for (int i = 1; i <= num_row; i++) {
        clear_row(MAXROW - i);
    }
}



// void printf(const char* str) {
//     static uint16_t* VideoMemory = (uint16_t*)0xb8000;
//     static uint8_t x = 0,y = 0;
//     for(int i = 0; str[i] != '\0'; i++) {
//         switch(str[i]) {
//             case '\n': {
//                 x = 0;
//                 y++;
//                 break;
//             } default: {
//                 VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
//                 x++;
//             }
//         }

//         if (x >= 80) {
//             x = 0;
//             y++;
//         }

//         if (y >= 25) {
//             clear_screen();
//             x = 0;
//             y = 0;
//         }
//     }
// }

// #define VGABASEADDR 0xB8000

// void putstr(const char* str) {
//     static uint16_t* VideoMemory = (uint16_t*)VGABASEADDR;
//     static uint8_t x = 0, y = 0;
//     for(int i = 0; str[i] != '\0'; i++) {
//         switch(str[i]) {
//             case '\n': {
//                 x = 0;
//                 y++;
//                 break;
//             } default: {
//                 VideoMemory[COL*y+x] = (VideoMemory[COL*y+x] & 0xFF00) | str[i];
//                 x++;
//             }
//         }

//         if (x >= COL) {
//             x = 0;
//             y++;
//         }

//         if (y >= ROW) {
//             // clear_screen();
//             strncpy((const char*)(VGABASEADDR + COL* 2), (char*)VGABASEADDR, (ROW - 1) * COL * 2);
//             memsetw((uint16_t*)(VGABASEADDR + (ROW - 1) * COL * 2), 0, COL);
//             x = 0;
//             y = ROW - 1;
//         }
//     }
// }


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
    char *p = (char *)(VIDEO_MEM + CHAR_OFFSET(cursor_row, cursor_col));
    uint8_t attr = ATTR(bg, fg);
    switch (c) {
        case '\a': {
            break;
        } case '\b': {
            break;
        } case '\t': {
            break;
        } case '\n': {
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
        // __asm__("xchg bx, bx");
        scroll(cursor_row - MAXROW + 1);
        cursor_row = MAXROW - 1;
    }
    set_cursor(cursor_row, cursor_col);
}

void init_screen() {
    clear_screen();
    set_cursor(0, 0);
}