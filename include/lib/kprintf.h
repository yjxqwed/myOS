#ifndef __KPRINTF_H__
#define __KPRINTF_H__

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
int kprintf(KP_LEVEL kpl, const char *fmt, ...);
int console_kprintf(KP_LEVEL kpl, const char *fmt, ...);

// @brief kernel sprintf, string format
int ksprintf(char *out, const char *fmt, ...);

#ifndef printf
#define printf(fmt, ...) kprintf(KPL_DEBUG, fmt, ##__VA_ARGS__)
#endif

// void kprintf_enable_paging();
// void kprintf_use_tty();

#endif
