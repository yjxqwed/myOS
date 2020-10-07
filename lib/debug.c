#include "debug.h"
#include "screen.h"
#include "string.h"

// #define DEBUG

void debugMagicBreakpoint() {
    #ifdef DEBUG
        __asm__ volatile ("xchg %bx, %bx");
    #endif
}

void printISRParam(const isrp_t* p) {
    printf(" eip=");
    char out[UINT32LEN];
    printf(uitosh(p->eip, out));
    printf(" errco=");
    printf(uitosh(p->err_code, out));
}
