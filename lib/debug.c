#include "debug.h"

// #define DEBUG

void debugMagicBreakpoint() {
    #ifdef DEBUG
        __asm__ volatile ("xchg %bx, %bx");
    #endif
}