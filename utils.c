#include "utils.h"

void debugMagicBreakpoint() {
    asm("xchg %bx, %bx");
}