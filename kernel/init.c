#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <driver/hd.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <kprintf.h>
#include <driver/pit.h>
#include <multiboot/multiboot.h>
#include <mm/pmem.h>
#include <mm/vmm.h>
#include <arch/x86.h>
#include <thread/thread.h>

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

static void test(void *args) {
    char *a = (char *)args;
    for (int i = 0; ; i++) {
        if (i % 1000000 == 0) {
            kprintf(KPL_DEBUG, " test: %s ", a);
            // print_ready_tasks();
            i = 0;
        }
    }
}

static void kmain(void *args) {
    kprintf(KPL_DEBUG, "main thread!\n");
    // thread_start("test1", 31, test, "abc");
    while (1);
}

// setup gdt, idt, etc
void kinit() {
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();
    kernel_init_paging();
    pmem_init();
    thread_init();
    thread_kmain();
    print_all_tasks();
    enable_int();
    thread_start("test1", 15, test, "abcde");
    thread_start("test2", 5, test, "hhhh");
    for (int i = 0; ; i++) {
        if (i % 1000000 == 0) {
            // print_ready_tasks();
            kprintf(KPL_DEBUG, " main ");
            i = 0;
        }
    }
    while (1);
}
