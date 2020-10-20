#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <driver/hd.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <sys/tss.h>
#include <kprintf.h>
#include <driver/pit.h>
#include <multiboot/multiboot.h>
#include <mm/pager.h>
#include <sys/interrupt.h>


void kernelMain(multiboot_info_t *mbi, uint32_t magic_number) {
    init_screen();


    // check magic number
    if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf(
            KPL_PANIC, "Invalid magic number 0x%x. System Halted.\n", 
            magic_number
        );
        while (1);
    }

    print_mem_info(mbi);
    print_hd_info();
    // setTssEntry0();
    timer_install(1000);
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    init_pd();
    debugMagicBreakpoint();

    kprintf(KPL_DUMP, "IF: %d\n", get_int_status());
    enable_int();
    kprintf(KPL_DUMP, "IF: %d\n", get_int_status());

    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    kprintf(KPL_DUMP, "%d is the minimum int32\n", (int32_t)0x80000000);
    kprintf(KPL_DUMP, "%d is the maximum int32\n", (int32_t)0x7FFFFFFF);
    kprintf(KPL_DUMP, "%d is zero in decimal\n", 0);
    kprintf(KPL_DUMP, "%x is zero in hexadecimal\n", 0);
    kprintf(KPL_DUMP, "%s is a test string\n", "#abcdefg$");
    kprintf(KPL_DUMP, "%c and %% are test characters\n", '@');
}
