#include <common/types.h>
#include <sys/gdt.h>
#include <device/screen.h>
#include <device/hd.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <kprintf.h>
#include <device/pit.h>
#include <device/kb.h>
#include <multiboot/multiboot.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <arch/x86.h>
#include <thread/thread.h>
#include <device/console.h>

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
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    kernel_init_paging();
    pmem_init();
    vmm_init();
    thread_init();
    thread_kmain();
    // print_all_tasks();
    timer_init(10000);
    kb_init();
    console_init();
    enable_int();
}
