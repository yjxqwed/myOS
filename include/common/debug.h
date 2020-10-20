#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <sys/isr.h>

void debugMagicBreakpoint();
void printISRParam(const isrp_t* p);

void panic_spin(
    const char* filename, int line,
    const char* funcname, const char* condition
);

#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

// NDEBUG = no debug

#define KDEBUG

#ifndef KDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) if (!(CONDITION)) { PANIC(#CONDITION); }
#endif

#endif
