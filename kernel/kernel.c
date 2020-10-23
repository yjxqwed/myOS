#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <driver/hd.h>
#include <common/debug.h>
#include <sys/idt.h>
// #include <sys/tss.h>
#include <kprintf.h>
#include <driver/pit.h>
#include <multiboot/multiboot.h>
#include <mm/pager.h>
#include <sys/interrupt.h>

void test_magic_number(uint32_t magic_number) {
    // check magic number
    if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf(
            KPL_PANIC, "Invalid magic number 0x%x. System Halted.\n", 
            magic_number
        );
        while (1);
    }
}


// void kernelMain(multiboot_info_t *mbi, uint32_t magic_number) {
void kernelMain() {
    clear_low_mem_mapping();
    // debugMagicBreakpoint();
    kprintf(KPL_DUMP, "KERNEL!\n");
    // while (1);
    // init_screen();



    // // print_mem_info(mbi);
    // mm_init(mbi);
    // // print_hd_info();
    // timer_install(1000);
    // setGlobalDescriptorTable();
    // mm_init(mbi);
    setInterruptDescriptorTable();
    // // init_pd();
    // // debugMagicBreakpoint();


    // debugMagicBreakpoint();
    // int i = 1 / 0;
    kprintf(KPL_DUMP, "IF: %d\n", get_int_status());
    enable_int();
    kprintf(KPL_DUMP, "IF: %d\n", get_int_status());

    while (1);

    // kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // kprintf(KPL_DUMP, "%d is the minimum int32\n", (int32_t)0x80000000);
    // // debugMagicBreakpoint();
    // kprintf(KPL_DUMP, "%d is the maximum int32\n", (int32_t)0x7FFFFFFF);
    // kprintf(KPL_DUMP, "%d is zero in decimal\n", 0);
    // kprintf(KPL_DUMP, "%x is zero in hexadecimal\n", 0);
    // kprintf(KPL_DUMP, "%s is a test string\n", "#abcdefg$");
    // kprintf(KPL_DUMP, "%c and %% are test characters\n", '@');
}
