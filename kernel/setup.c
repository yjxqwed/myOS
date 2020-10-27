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
#include <mm/mem.h>
#include <sys/interrupt.h>

static void test_magic_number(uint32_t magic_number) {
    // check magic number
    if (magic_number != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf(
            KPL_PANIC, "Invalid magic number 0x%x. System Halted.\n", 
            magic_number
        );
        while (1);
    }
}

// setup paging
void ksetup_before_paging(multiboot_info_t *mbi, uint32_t magic_number) {
    test_magic_number(magic_number);
    mm_init(mbi);
}

// setup gdt, idt, etc
void ksetup_after_paging() {
    clear_low_mem_mapping();
    video_mem_enable_paging();
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    enable_int();
}