#include <common/types.h>
#include <sys/gdt.h>
#include <device/screen.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <kprintf.h>
#include <device/pit.h>
#include <device/kb.h>
#include <device/ata.h>
#include <multiboot/multiboot.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <arch/x86.h>
#include <thread/thread.h>
#include <sys/syscall.h>

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

// test magic number and check the size of physical memory installed
// install the pre-mapped for temp use
// initialize boot_alloc for setup kernel's real physical page allocator
void entry_setup(multiboot_info_t *mbi, uint32_t magic_number) {
    test_magic_number(magic_number);
    // get information of the physical memory installed
    detect_memory(mbi);
    // install page directory for bootstrap
    install_boot_pg();
    // change video mem to vaddr
    video_mem_enable_paging();
}



// setup gdt, idt, etc
void kinit() {
    // setup new gdt
    setGlobalDescriptorTable();
    // setup idt
    setInterruptDescriptorTable();
    // init pagging system
    kernel_init_paging();
    // init physical memory management
    pmem_init();
    // init virtual memory management
    vmm_init();
    // init thread
    thread_init();
    // init syscall
    syscall_init();


    // init device screen
    init_screen();
    // init device pit
    timer_init(10000);
    // init device keyboard
    kb_init();
    // enable interrupt
    enable_int();
    // init hard drives
    ata_init();
}
