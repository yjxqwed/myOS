// #include "types.h"
// #include "gdt.h"
// #include "screen.h"
// #include "debug.h"
// #include "idt.h"
// #include "tss.h"
#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <sys/tss.h>
#include <kprintf.h>

void kernelMain(void) {
    // clear_screen();
    init_screen();
    // debugMagicBreakpoint();
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang (before my gdt)\n");
    setTssEntry0();
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    // debugMagicBreakpoint();
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang (after my gdt)\n");
    // debugMagicBreakpoint();

    // to allow the interrupt
    // __asm__ volatile(
    //     "sti"
    //     "\n\thlt"
    // );

    
    // debugMagicBreakpoint();
    // int b = 1 / 0;
}
