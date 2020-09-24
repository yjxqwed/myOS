#include "types.h"
#include "screen.h"

void clear_screen() {
    volatile int8_t* video_buffer = (volatile int8_t*)0xB8000;
    const unsigned int total = COL * ROW * 2;
    int i;
    while(i < total)
    {
        video_buffer[i++] = ' ';
        video_buffer[i++] = 0x0f;              
    }
}

void printf(const char* str) {
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    static uint8_t x = 0,y = 0;
    for(int i = 0; str[i] != '\0'; i++) {
        switch(str[i]) {
            case '\n': {
                x = 0;
                y++;
                break;
            } default: {
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
            }
        }

        if (x >= 80) {
            x = 0;
            y++;
        }

        if (y >= 25) {
            clear_screen();
            x = 0;
            y = 0;
        }
    }
}