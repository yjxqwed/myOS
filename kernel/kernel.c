#include "types.h"
#include "gdt.h"
#include "screen.h"
#include "debug.h"
#include "idt.h"
#include "tss.h"

void kernelMain(void) {
    clear_screen();
    // debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (before my gdt)\n");
    setTssEntry0();
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    // debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (after my gdt)\n");
    // debugMagicBreakpoint();

    // to allow the interrupt
    // __asm__ volatile(
    //     "sti"
    //     "\n\thlt"
    // );

    
    // debugMagicBreakpoint();
    // int b = 1 / 0;
}
