#ifndef __KPRINTF_H__
#define __KPRINTF_H__

#include <driver/screen.h>

typedef enum KP_LEVEL {
    KPL_DUMP,   // black bg, white fg
    KPL_NOTICE, // blue bg, white fg
    KPL_DEBUG,  // cyan bg, light green fg
    KPL_PANIC,  // red bg, yellow fg
    KPL_SIZE
} KP_LEVEL;


// kernel printf
// DONT be too long, please
void kprintf(KP_LEVEL kpl, const char *fmt, ...);


#endif
