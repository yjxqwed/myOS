#include <mm/mem.h>
#include <mm/pager.h>
#include <bitmap.h>
#include <kprintf.h>

// the memory pool is page (4 KiB) granular and a continuous region
// @members:
//  btmp is the bitmap for acclocating pages
//  num_total_pages is the total amount of pages in this pool
//  start_page_number is the starting page's index of this pool
typedef struct MemoryPool {
    btmp_t btmp;
    uint32_t num_total_pages;
    uint32_t start_page_number;
} MemoryPool;

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))
void print_mem_info(multiboot_info_t *mbi) {
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
    kprintf(KPL_NOTICE, "=============================================\n");
}

void mm_init(multiboot_info_t *mbi) {
    if (CHECK_FLAG(mbi->flags, 0)) {
        uint32_t high_free_mem = mbi->mem_upper * 1024;
        if (high_free_mem < MEM_LIMIT) {
            kprintf(
                KPL_PANIC, 
                "Not Enough Memory; "
                "try with >= 256 MiB mem. System Halted.\n"
            );
            while (1);
        }
        uint32_t high_free_pages = high_free_mem / PAGE_SIZE;
        kprintf(
            KPL_NOTICE,
            "Free mem: 0x%X byte(s); "
            "free pages: 0x%X page(s).\n",
            high_free_mem, high_free_pages
        );
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
    init_paging();
}