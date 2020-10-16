#include <common/types.h>
#include <sys/gdt.h>
#include <driver/screen.h>
#include <common/debug.h>
#include <sys/idt.h>
#include <sys/tss.h>
#include <kprintf.h>
#include <driver/pit.h>
#include <multiboot/multiboot.h>
#include <sys/interrupt.h>

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))

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

    kprintf(KPL_NOTICE, "================ System Info ================\n");
    kprintf(KPL_NOTICE, "Flags = 0x%x\n", (uint32_t)mbi->flags);

    // check mem
    if (CHECK_FLAG(mbi->flags, 0)) {
        kprintf(
            KPL_NOTICE, "mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
            (uint32_t)mbi->mem_lower, (uint32_t)mbi->mem_upper
        );
    }

    // check mmap
    if (CHECK_FLAG (mbi->flags, 6)) {
        multiboot_memory_map_t *mmap;
        kprintf(
            KPL_NOTICE, "mmap_addr = 0x%X, mmap_length = 0x%X\n",
            (uint32_t)mbi->mmap_addr, (uint32_t)mbi->mmap_length
        );
        for (
            mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
            (uint32_t)mmap < mbi->mmap_addr + mbi->mmap_length;
            mmap = (multiboot_memory_map_t *)(
                (uint32_t)mmap + mmap->size + sizeof(mmap->size)
            )
        ) {
            kprintf(
                KPL_NOTICE, 
                " size = 0x%x, base_addr = 0x%X,"
                " length = 0x%X, type = 0x%x\n",
                (uint32_t) mmap->size,
                // (uint32_t) (mmap->addr >> 32),
                (uint32_t) (mmap->addr & 0xffffffff),
                // (uint32_t) (mmap->len >> 32),
                (uint32_t) (mmap->len & 0xffffffff),
                (uint32_t) mmap->type)
            ;
        }
    }
    kprintf(KPL_NOTICE, "=============================================\n");

    setTssEntry0();
    timer_install(1000);
    setGlobalDescriptorTable();
    setInterruptDescriptorTable();

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
