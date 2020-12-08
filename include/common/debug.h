#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <sys/isr.h>

void printISRParam(const isrp_t* p);

void panic_spin(
    const char* filename, int line,
    const char* funcname, const char* cause
);

#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define KDEBUG

#ifndef KDEBUG
#define MAGICBP ((void)0)
#else
#define MAGICBP do { __asm__ volatile("xchg bx, bx"); } while (0);
#endif

#ifndef KDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) if (!(CONDITION)) { PANIC(#CONDITION); }
#endif

#endif
