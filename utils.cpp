#include "utils.h"

void MagicBreakpoint() {
    asm("xchg bx, bx");
}