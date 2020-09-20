#include "types.h"
#include "gdt.h"
#include "screen.h"
#include "utils.h"

void kernelMain(void* multiboot_structure, unsigned int magic_number) {
    clear_screen();
    debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (before my gdt)\n");
    setGlobalDescriptorTable();
    debugMagicBreakpoint();
    printf("Hello Wolrd! --- This is myOS by Justing Yang (after my gdt)\n");
    while (1);
}