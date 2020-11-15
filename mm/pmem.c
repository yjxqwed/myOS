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
static uint32_t nppages = 0;

// the array of ppage_t, this array is used for page allocating
static ppage_t *pmap = NULL;
static ppage_t *free_pages_list = NULL;

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

void setup_memory(multiboot_info_t *mbi) {
    print_mem_info(mbi);
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
        max_high_pfn = __page_number(HIGH_MEM_BASE + high_free_mem - 1);
        // min_high_pfn is after the kernel image
        min_high_pfn = __page_number(_end) + 2;
        nppages = max_high_pfn + 1;
        kprintf(KPL_NOTICE, "minpfn=0x%x, maxpfn=0x%x, npages=0x%x\n", min_high_pfn, max_high_pfn, nppages);
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
}

// The allocator is only for allocating pages for initializing
// mm structures
// On success, return the virtual address; NULL otherwise.
void *boot_alloc(uint32_t n, bool page_alligned) {

    static uintptr_t next_free_byte = NULL;

    // init at the very first call
    if (next_free_byte == NULL) {
        next_free_byte = min_high_pfn * PAGE_SIZE;
    }

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
    void *va = (void *)__va(addr);
    memset(va, 0, n);
    return va;
}

static void print_page_t(ppage_t *p) {
    ASSERT(p != NULL);
    kprintf(
        KPL_DEBUG, "page_t(0x%X){link=0x%X, ref=%d}\n",
        (uintptr_t)p, p->next_free_ppage, p->num_ref
    );
}

static inline void *page2kva(ppage_t *p) {
    return (void *)((p - pmap) * PAGE_SIZE);
}

static inline void *page2pa(ppage_t *p) {
    return __pa(page2kva(p));
}

// init the pmem management structures
void pmem_init() {
    // initialize pmap
    pmap = boot_alloc(sizeof(ppage_t) * nppages, false);

    // fpn is the next page after pages allocated by boot_alloc
    // pages after fpn may be used in further operations
    int fpn = __page_number(__pa(boot_alloc(0, true)));
    ASSERT(fpn < max_high_pfn);
    kprintf(KPL_DEBUG, "pmap=0x%X\n", pmap);
    kprintf(KPL_DEBUG, "nppages=0x%x\nfree_page=0x%x\n", nppages, fpn);
    // These pages will never be freed or reused!
    for (int i = 0; i < fpn; i++) {
        pmap[i].next_free_ppage = NULL;
        pmap[i].num_ref = 1;
    }
    for (int i = fpn; i <= max_high_pfn; i++) {
        pmap[i].next_free_ppage = free_pages_list;
        pmap[i].num_ref = 0;
        free_pages_list = &(pmap[i]);
    }
}

ppage_t *page_alloc(uint32_t gfp_flags) {
    if (free_pages_list == NULL) {
        return NULL;
    }
    ppage_t *fp = free_pages_list;
    free_pages_list = free_pages_list->next_free_ppage;
    fp->next_free_ppage = NULL;
    if (gfp_flags & GFP_ZERO) {
        memset(page2kva(fp), 0, PAGE_SIZE);
    }
    return fp;
}