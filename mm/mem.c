#include <mm/mem.h>
#include <mm/pager.h>
#include <bitmap.h>
#include <kprintf.h>
#include <common/types.h>
#include <sys/global.h>

#define __page_number(x) (x / PAGE_SIZE)

// the memory page pool is page (4 KiB) granular and a continuous region
// @members:
//  btmp is the bitmap for acclocating pages
//  num_total_pages is the total amount of pages in this pool
//  start_page_number is the starting page's index of this pool
typedef struct MemoryPagePool {
    btmp_t btmp;
    uint32_t num_total_pages;
    uint32_t start_page_number;
} mpp_t;

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

// not always return
// if pass memory check, return number of high free pages (from 0x00100000)
static uint32_t check_memory(multiboot_info_t *mbi) {
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
        uint32_t high_free_pages = __page_number(high_free_mem);
        kprintf(
            KPL_NOTICE,
            "Free mem: 0x%X byte(s); "
            "free pages: 0x%X page(s).\n",
            high_free_mem, high_free_pages
        );
        return high_free_pages;
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
    // never reach here
    return 0;
}

// kernel physical memory page pool
static mpp_t kppool;
// user physical memory page pool
static mpp_t uppool;

// kernel virtual memory page pool
static mpp_t kvpool;

static void init_pool(
    mpp_t *pool, uint32_t start_page_no,
    uint32_t num_pages, uint32_t btmp_base
) {
    pool->num_total_pages = num_pages;
    pool->start_page_number = start_page_no;
    pool->btmp.bits_ = (uint8_t *)btmp_base;
    // TODO: what if num_pages % 8 != 0?
    bitmap_init(&(pool->btmp), num_pages / 8);
}

static void print_pool(const mpp_t *pool, const char* name) {
    kprintf(
        KPL_DUMP,
        "%s: { start_page_number = 0x%x, num_total_pages = 0x%x, "
        "btmp = { byte_num = 0x%x, bits = 0x%X } }\n",
        name, pool->start_page_number, pool->num_total_pages,
        pool->btmp.byte_num_, (uint32_t)pool->btmp.bits_
    );
}

// init the pmem pools
// @param num_pages is the total page from 0x00100000
static void init_ppools(uint32_t num_free_pages) {
    uint32_t num_pages = num_free_pages + AVAIL_PAGE_BASE;
    uint32_t half_num_pages = num_pages / 2;
    uint32_t k_free_pages = 0;
    uint32_t k_needed_pages = __page_number(K_NEEDED_MEM);
    uint32_t free_page_no = __page_number(FREE_MEM_BASE);
    if (half_num_pages > k_needed_pages) {
        k_free_pages = k_needed_pages - free_page_no;
    } else {
        k_free_pages = half_num_pages - free_page_no;
    }
    uint32_t u_free_pages = num_pages - free_page_no - k_free_pages;
    kprintf(KPL_NOTICE, "\nkfp: %x; ufp: %x\n", k_free_pages, u_free_pages);
    init_pool(&kppool, free_page_no, k_free_pages, KPPOOL_BTMP_BASE_ADDR);
    print_pool(&kppool, "kernel ppool");
    init_pool(
        &uppool, free_page_no + k_free_pages,
        u_free_pages, UPPOOL_BTMP_BASE_ADDR
    );
    print_pool(&uppool, "user ppool");
}

void mm_init(multiboot_info_t *mbi) {
    print_mem_info(mbi);
    uint32_t num_pages = check_memory(mbi);
    init_ppools(num_pages);
    init_paging();
}