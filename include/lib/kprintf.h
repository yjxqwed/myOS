#ifndef __KPRINTF_H__
#define __KPRINTF_H__

#include <device/screen.h>

typedef enum KP_LEVEL {
    KPL_DUMP,   // black bg, white fg
    KPL_NOTICE, // blue bg, white fg
    KPL_DEBUG,  // cyan bg, light green fg
    KPL_PANIC,  // red bg, yellow fg
    KPL_SIZE
} KP_LEVEL;


// kernel printf
// DONT be too long, please (1024 char max)
// %s -> string
// %d -> signed decimal int32
// %x -> unsigned hex uint32
// %X -> unsigned hex uint32 full 8 bits
// %c -> char
void kprintf(KP_LEVEL kpl, const char *fmt, ...);


#endif
