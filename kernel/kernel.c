#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <sys/tss.h>
#include <kprintf.h>
#include <driver/pit.h>

void kernelMain(void) {
    init_screen();

    setTssEntry0();
    timer_install(1000);
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();

    // to allow the interrupt
    __asm__ volatile(
        "sti"
        "\n\thlt"
    );

    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    kprintf(KPL_DUMP, "%d is the minimum int32\n", (int32_t)0x80000000);
    kprintf(KPL_DUMP, "%d is the maximum int32\n", (int32_t)0x7FFFFFFF);
    kprintf(KPL_DUMP, "%d is zero in decimal\n", 0);
    kprintf(KPL_DUMP, "%x is zero in hexadecimal\n", 0);
    kprintf(KPL_DUMP, "%s is a test string\n", "#abcdefg$");
    kprintf(KPL_DUMP, "%c and %% are test characters\n", '@');
}
