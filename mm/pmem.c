#include <mm/pmem.h>
#include <bitmap.h>
#include <kprintf.h>
#include <common/types.h>
#include <common/debug.h>
#include <string.h>
#include <myos.h>
#include <arch/x86.h>

extern void *kernel_image_end;
// _end is the (physical) end of the kernel binary image
static uintptr_t _end = __pa(&kernel_image_end);

uint32_t max_high_pfn = 0;
static uint32_t min_high_pfn = 0;

// the array of ppage_t, this array is used for page allocating
static ppage_t *pmap = NULL;

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))
static void print_mem_info(multiboot_info_t *mbi) {
    kprintf(KPL_NOTICE, "================ Memory Info ================\n");
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
        kprintf(
            KPL_NOTICE, "mmap_addr = 0x%X, mmap_length = 0x%X\n",
            (uint32_t)mbi->mmap_addr, (uint32_t)mbi->mmap_length
        );
        for (
            multiboot_memory_map_t *mmap =
                (multiboot_memory_map_t *)mbi->mmap_addr;

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
                (uint32_t) mmap->type
            );
        }
    }
    kprintf(KPL_NOTICE, "        _end=0x%X\n", _end);
    kprintf(KPL_NOTICE, "=============================================\n");
}

// not always return
// if pass memory check, return the max page frame number
static uint32_t check_memory(multiboot_info_t *mbi) {
    if (CHECK_FLAG(mbi->flags, 0)) {
        // mbi->mem_upper is given in KiB
        uint32_t high_free_mem = mbi->mem_upper * 1024;
        if (high_free_mem < MIN_MEMORY_LIMIT) {
            kprintf(
                KPL_PANIC, 
                "Not Enough Memory; "
                "try with >= 256 MiB mem. System Halted.\n"
            );
            while (1);
        }
        uint32_t max_pfn = __page_number(HIGH_MEM_BASE + high_free_mem - 1);
        return max_pfn;
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
    // never reach here
    return 0;
}

// The allocator is only for allocating pages for initializing
// mm structures
// On success, return the virtual address; NULL otherwise.
static void *boot_alloc(uint32_t n, bool page_alligned) {

    static uintptr_t next_free_byte = NULL;

    // init at the first call
    if (next_free_byte == NULL) {
        next_free_byte = min_high_pfn * PAGE_SIZE;
    }

    // ASSERT(next_free_byte + n <= (max_high_pfn + 1) * PAGE_SIZE);
    if (next_free_byte + n > (max_high_pfn + 1) * PAGE_SIZE) {
        return NULL;
    }

    uintptr_t addr = NULL;

    if (page_alligned) {
        addr = __pg_start_addr(next_free_byte);
        if (next_free_byte != addr) {
            addr += PAGE_SIZE;
        }
    } else {
        addr = next_free_byte;
    }
    next_free_byte = addr + n;
    return (void *)__va(addr);
}

void setup_memory(multiboot_info_t *mbi) {
    print_mem_info(mbi);
    max_high_pfn = check_memory(mbi);
    // min_high_pfn is after the kernel image
    min_high_pfn = __page_number(_end) + 2;

    kprintf(
        KPL_NOTICE, "min_hign_pfn = 0x%x, max_high_pfn = 0x%x\n",
        min_high_pfn, max_high_pfn
    );

    void *a = boot_alloc(0, false);
    void *b = boot_alloc(29, false);
    void *c = boot_alloc(31, false);
    void *d = boot_alloc(13, true);
    void *e = boot_alloc(PAGE_SIZE, true);
    void *f = boot_alloc(0, false);
    void *g = boot_alloc(0x40000000, true);

    kprintf(KPL_DEBUG, "a=0x%X\n", (uintptr_t)a);
    kprintf(KPL_DEBUG, "b=0x%X\n", (uintptr_t)b);
    kprintf(KPL_DEBUG, "c=0x%X\n", (uintptr_t)c);
    kprintf(KPL_DEBUG, "d=0x%X\n", (uintptr_t)d);
    kprintf(KPL_DEBUG, "e=0x%X\n", (uintptr_t)e);
    kprintf(KPL_DEBUG, "f=0x%X\n", (uintptr_t)f);
    kprintf(KPL_DEBUG, "g=0x%X\n", (uintptr_t)g);
}