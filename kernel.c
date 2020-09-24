#include "types.h"
#include "gdt.h"
#include "screen.h"
#include "debug.h"
#include "idt.h"

void kernelMain(void* multiboot_structure, unsigned int magic_number) {
    clear_screen();
    // debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (before my gdt)\n");
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    // debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (after my gdt)\n");
    debugMagicBreakpoint();
    int b = 1 / 0;
    // while (1);
}