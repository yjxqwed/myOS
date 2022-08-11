#include <common/types.h>
#include <sys/gdt.h>
#include <device/screen.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <lib/kprintf.h>
#include <device/pit.h>
#include <device/tty.h>
// #include <device/ata.h>
#include <device/ide.h>
#include <multiboot/multiboot.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <arch/x86.h>
#include <thread/thread.h>
#include <sys/syscall.h>
// #include <fs/myfs/fs.h>
#include <fs/simplefs/simplefs.h>

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
// install the pre-mapped PDs and PTs for temp use
// initialize boot_alloc for setup kernel's real physical page allocator
void entry_setup(multiboot_info_t *mbi, uint32_t magic_number) {
    test_magic_number(magic_number);
    // get information of the physical memory installed
    detect_memory(mbi);
    // install page directory for bootstrap
    install_boot_pg();
    // change video mem to vaddr
    video_mem_enable_paging();
    // kprintf_enable_paging();
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
    // init device pit
    timer_init(10000);
    // init device hd
    // ata_init();
    ata_init_2();


    // char *buf = kmalloc(16 * SECTOR_SIZE);
    // ASSERT(buf != NULL);

    // extern disk_t *first_disk;
    // ata_read(first_disk, 2049, buf, 1);
    // kprintf(KPL_DEBUG, "2049: 0x%X\n", *(uint32_t *)buf);
    // ata_read(first_disk, 2306, buf, 16);
    // kprintf(KPL_DEBUG, "2306: 0x%X\n", *(uint32_t *)buf);
    // ata_read(first_disk, 2050, buf, 1);
    // kprintf(KPL_DEBUG, "2050: 0x%X\n", *(uint32_t *)buf);
    // ata_read(first_disk, 2050, buf, 1);
    // kprintf(KPL_DEBUG, "2050: 0x%X\n", *(uint32_t *)buf);

    // ide_read_sectors(1, 1, 2049, (uint32_t)buf);
    // printf("2049: %X\n", *(uint32_t*)buf);

    // ide_read_sectors(1, 16, 2306, (uint32_t)buf);
    // printf("2306: %X\n", *(uint32_t*)buf);

    // ide_read_sectors(1, 1, 2050, (uint32_t)buf);
    // printf("2050: %X\n", *(uint32_t*)buf);

    // ide_read_sectors(1, 1, 2050, (uint32_t)buf);
    // printf("2050: %X\n", *(uint32_t*)buf);

    // while (1);

    // init tty
    tty_init();
    // kprintf_use_tty();
    // enable interrupt
    enable_int();
    // init hard drives
    
    // init file system
    // fs_init();
    simplefs_init();
}
