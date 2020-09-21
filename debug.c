#include "debug.h"

void debugMagicBreakpoint() {
    asm("xchg %bx, %bx");
}