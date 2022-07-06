#ifndef __UTILS_H__
#define __UTILS_H__

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define ASSERT assert
#define kmalloc malloc
#define kfree free
#define ksprintf sprintf
#define kprintf(kpl, fmt, ...) printf(fmt, ##__VA_ARGS__)

typedef enum KP_LEVEL {
    KPL_DUMP,   // black bg, white fg
    KPL_NOTICE, // blue bg, white fg
    KPL_DEBUG,  // cyan bg, light green fg
    KPL_PANIC,  // red bg, yellow fg
    KPL_SIZE
} KP_LEVEL;

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// round up of a / b
#define ROUND_UP_DIV(a, b) (((a) + (b) - 1) / (b))
#define ROUND_UP(a, b) (ROUND_UP_DIV(a, b) * b)

#define __attr_packed __attribute__((packed))

#define PANIC(msg) do {printf(msg); exit(1);} while(0);

#define memcpy(src, dst, size) memcpy(dst, src, size)
#define strcpy(src, dst) strcpy(dst, src)

#endif
