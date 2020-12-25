#include <mm/pmem.h>
#include <bitmap.h>
#include <kprintf.h>
#include <common/types.h>
#include <common/debug.h>
#include <string.h>
#include <myos.h>
#include <arch/x86.h>
#include <thread/sync.h>
#include <bitmap.h>
#include <common/utils.h>

// the page directory used by kernel
static pte_t *kern_pg_dir = NULL;

// the max free page frame number
static uint32_t max_high_pfn = 0;
// the min free page frame number (above 1MiB)
static uint32_t min_high_pfn = 0;

// free page frame number (after mem used by boot_alloc)
static uint32_t free_pfn = 0;


// total memory installed (in megabytes)
static uint32_t total_mem_mb = 0;
// total physical pages
static uint32_t nppages = 0;

static ppage_t *zpage = NULL;


/***********************************************************
 * The machine is installed with physical memory size of N *
 *                                                         *
 * |<---- nppages/total_mem_mb ------>|                    *
 * 0  1MB                             N                    *
 * |---|------|------|------------|---|                    *
 *   R    KI   ^ BA   ^          ^  R                      *
 *             |      |          |                         *
 *             |      |          +----- max_high_pfn       *
 *             |      +---------------- free_pfn           *
 *             +----------------------- min_high_pfn       *
 *                                                         *
 *  R: reserved                                            *
 *  KI: kernel image                                       *
 *  BA: mem used by boot_alloc                             *
 *                                                         *
 *  pages in [free_pfn, max_high_pfn] are free of use      *
 ***********************************************************/

// the array of ppage_t,
static ppage_t *pmap = NULL;
// the bitmap used for page allocating
static btmp_t pmem_btmp;

#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))
static void analyze_mem_map(multiboot_info_t *mbi) {
    kprintf(KPL_NOTICE, "================ Memory Info ================\n");
    kprintf(KPL_NOTICE, "Flags = 0x%x\n", (uint32_t)mbi->flags);

    // check mem
    if (CHECK_FLAG(mbi->flags, 0)) {
        kprintf(
            KPL_NOTICE, "mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
            (uint32_t)mbi->mem_lower, (uint32_t)mbi->mem_upper
        );
    }

    uint32_t num_pages = 0x100000 / 0x1000;

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
                (uint32_t)(mmap->size),
                // (uint32_t) (mmap->addr >> 32),
                (uint32_t)(mmap->addr),
                // (uint32_t) (mmap->len >> 32),
                (uint32_t)(mmap->len),
                (uint32_t)(mmap->type)
            );
            uint32_t base = mmap->addr;
            uint32_t len = mmap->len;
            if (base >= 0x100000) {
                if ((base / 0x1000) == num_pages) {
                    num_pages = base / 0x1000 + len / 0x1000;
                }
            }
        } 
        // 1MiB = 0x100 pages
        nppages = num_pages;
        total_mem_mb = nppages / 0x100;
        if (total_mem_mb > MAX_MEMORY_LIMIT_MB) {
            total_mem_mb = MAX_MEMORY_LIMIT_MB;
            nppages = total_mem_mb * 0x100;
        }
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
    kprintf(KPL_NOTICE, "total mem installed = 0x%x MiB\n", total_mem_mb);
    kprintf(KPL_NOTICE, "=============================================\n");
}

void detect_memory(multiboot_info_t *mbi) {
    analyze_mem_map(mbi);
    if (CHECK_FLAG(mbi->flags, 0)) {
        // mbi->mem_upper is given in KiB
        uint32_t high_free_mem = mbi->mem_upper * 1024;
        if (total_mem_mb < MIN_MEMORY_LIMIT_MB) {
            kprintf(
                KPL_PANIC, 
                "Not Enough Memory; "
                "try with >= 128 MiB mem. System Halted.\n"
            );
            while (1);
        }
        if (high_free_mem > (MAX_MEMORY_LIMIT_MB  - 1) * 0x100000) {
            high_free_mem = (MAX_MEMORY_LIMIT_MB  - 1) * 0x100000;
        }
        max_high_pfn = __page_number(HIGH_MEM_BASE + high_free_mem - 1);
        // min_high_pfn is after the kernel image
        extern void *kernel_image_end;
        // _end is the (physical) end of the kernel binary image
        uintptr_t _end = __pa(&kernel_image_end);
        min_high_pfn = __page_number(_end) + 2;
        kprintf(
            KPL_NOTICE, "minpfn=0x%x, maxpfn=0x%x, npages=0x%x\n",
            min_high_pfn, max_high_pfn, nppages
        );
    } else {
        kprintf(KPL_PANIC, "No Memory Info. System Halted.\n");
        while (1);
    }
}

// The allocator is only for allocating pages for initializing
// mm structures
// On success, return the kernel virtual address; NULL otherwise.
// @param n number of bytes required
// @param page_alligned whether the block should be page alligned
static void *boot_alloc(uint32_t n, bool_t page_alligned) {

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
        addr = ROUND_UP_DIV(next_free_byte, PAGE_SIZE) * PAGE_SIZE;
    } else {
        addr = next_free_byte;
    }
    next_free_byte = addr + n;
    void *va = (void *)__va(addr);
    memset(va, 0, n);
    return va;
}

// page to kernel virtual address
void *page2kva(ppage_t *p) {
    return __va(page2pa(p));
}

// page to physical address
void *page2pa(ppage_t *p) {
    return (p - pmap) * PAGE_SIZE;
}

ppage_t *kva2page(void *kva) {
    return pa2page(__pa(kva));
}

ppage_t *pa2page(void *pa) {
    return pmap + (uintptr_t)pa / PAGE_SIZE;
}

static mutex_t pmem_lock;

static ppage_t *__pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags);
static void __page_incref(ppage_t *p);

void print_page(ppage_t *p) {
    if (p == NULL) {
        kprintf(KPL_DEBUG, "NULL");
    }
    mutex_lock(&(p->page_lock));
    kprintf(
        KPL_DEBUG,
        "{kva=0x%X, num_ref=%d}",
        page2kva(p), p->num_ref
    );
    mutex_unlock(&(p->page_lock));
}

// init the pmem management structures
void pmem_init() {
    mutex_init(&pmem_lock);
    // initialize pmap
    pmap = boot_alloc(sizeof(ppage_t) * nppages, False);
    int byte_len = nppages / 8;
    pmem_btmp.bits_ = boot_alloc(byte_len, False);
    bitmap_init(&pmem_btmp, byte_len);
    // fpn is the next page after pages allocated by boot_alloc
    // pages after fpn may be used in further operations
    int fpn = __page_number(__pa(boot_alloc(0, True)));
    free_pfn = fpn;
    ASSERT(fpn < max_high_pfn);
    kprintf(KPL_DEBUG, "nppages=0x%x\nfree_page=0x%x\n", nppages, fpn);
    // These pages will never be freed or reused!
    for (int i = 0; i < fpn; i++) {
        pmap[i].num_ref = 0;
        mutex_init(&(pmap[i].page_lock));
        bitmap_set(&pmem_btmp, i, 1);
    }
    // Free pages. Free for future use.
    for (int i = fpn; i <= max_high_pfn; i++) {
        pmap[i].num_ref = 0;
        mutex_init(&(pmap[i].page_lock));
    }
    // These pages are reserved by machine (known from mutiboot memory map)
    for (int i = max_high_pfn + 1; i < nppages; i++) {
        pmap[i].num_ref = 0;
        mutex_init(&(pmap[i].page_lock));
        bitmap_set(&pmem_btmp, i, 1);
    }
    kprintf(KPL_DEBUG, "pmap=0x%X\n", pmap);
    kprintf(KPL_DEBUG, "pmem_btmp=");
    print_btmp(&pmem_btmp);
    kprintf(KPL_DEBUG, "\n");

    zpage = __pages_alloc(1, GFP_ZERO);
    __page_incref(zpage);

}


ppage_t *get_zpage() {
    return zpage;
}


static ppage_t *__pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags) {
    if (pg_cnt == 0) {
        return NULL;
    }
    int idx = bitmap_scan(&pmem_btmp, pg_cnt);
    if (idx == -1) {
        return NULL;
    }
    for (int i = 0, j = idx; i < pg_cnt; i++, j++) {
        bitmap_set(&pmem_btmp, j, 1);
    }
    ppage_t *fp = &(pmap[idx]);
    if (gfp_flags & GFP_ZERO) {
        memset(page2kva(fp), 0, PAGE_SIZE * pg_cnt);
    }
    return fp;
}


ppage_t *pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags) {
    ppage_t *fp = NULL;
    mutex_lock(&pmem_lock);
    fp = __pages_alloc(pg_cnt, GFP_ZERO);
    mutex_unlock(&pmem_lock);
    return fp;
}


static void __page_incref(ppage_t *p) {
    ASSERT(p != NULL);
    if (bitmap_bit_test(&pmem_btmp, p - pmap) == 0) {
        PANIC("incref a free page");
    }

    (p->num_ref)++;
}


void page_incref(ppage_t *p) {
    ASSERT(p != NULL);
    mutex_lock(&(p->page_lock));

    mutex_lock(&pmem_lock);
    if (bitmap_bit_test(&pmem_btmp, p - pmap) == 0) {
        PANIC("incref a free page");
    }
    mutex_unlock(&pmem_lock);

    (p->num_ref)++;
    mutex_unlock(&(p->page_lock));
}

void page_decref(ppage_t *p) {
    ASSERT(p != NULL);
    mutex_lock(&(p->page_lock));
    ASSERT(p->num_ref > 0);
    mutex_lock(&pmem_lock);
    if (bitmap_bit_test(&pmem_btmp, p - pmap) == 0) {
        PANIC("incref a free page");
    }
    mutex_unlock(&pmem_lock);
    (p->num_ref)--;
    if (p->num_ref == 0) {
        mutex_lock(&pmem_lock);
        bitmap_set(&pmem_btmp, (p - pmap), 0);
        mutex_unlock(&pmem_lock);
    }
    mutex_unlock(&(p->page_lock));
}

void pmem_print() {
    INT_STATUS old_status = disable_int();
    mutex_lock(&pmem_lock);
    kprintf(KPL_DUMP, "\n");
    print_btmp(&pmem_btmp);
    kprintf(KPL_DUMP, "\n");
    mutex_unlock(&pmem_lock);
    set_int_status(old_status);
}


/**
 *  ===========================================================
 *  ==================== Paging Management ====================
 *  ===========================================================
 */

// For bootstrap paging use
__PAGE_ALLIGNED pde_t boot_pg_dir[NRPDE];
__PAGE_ALLIGNED pte_t boot_pg_tab[NRPTE];

void install_boot_pg(void) {
    pte_t *ppg = (pte_t *)__pa(boot_pg_tab);
    for (int i = 0; i < NRPTE; i++) {
        ppg[i] = (pte_t)__pg_entry(i * PAGE_SIZE, PTE_PRESENT | PTE_WRITABLE);
    }
    memset(boot_pg_dir, 0, PAGE_SIZE);
    pde_t pde = (pde_t)__pg_entry(
        __pa(boot_pg_tab), PTE_PRESENT | PTE_WRITABLE
    );
    pde_t *ppd = (pde_t *)__pa(boot_pg_dir);
    ppd[0] = pde;
    ppd[__pde_idx(KERNEL_BASE)] = pde;

    // load the address of boot pg_dir to cr3
    lcr3(__pa(boot_pg_dir));
    // enable paging
    uint32_t cr0 = scr0();
    cr0 |= CR0_PG;
    lcr0(cr0);
}


void kernel_init_paging() {
    kern_pg_dir = boot_alloc(PAGE_SIZE, True);
    kprintf(KPL_DEBUG, "kern_pg_dir=0x%X\n", (uintptr_t)kern_pg_dir);
    int pfn = 0;
    uintptr_t va = __va((nppages - 1) * PAGE_SIZE);
    kprintf(KPL_DEBUG, "va=0x%X\n", va);
    for (
        int i = __pde_idx(KERNEL_BASE);
        i < __pde_idx(va);
        i++
    ) {
        pte_t *pg_tab = boot_alloc(PAGE_SIZE, True);
        ASSERT(pg_tab != NULL);
        for (int j = 0; j < NRPTE; j++, pfn++) {
            pg_tab[j] = (pte_t)__pg_entry(
                pfn * PAGE_SIZE, PTE_USER | PTE_PRESENT | PTE_WRITABLE
            );
        }
        kern_pg_dir[i] = (pde_t)__pg_entry(
            __pa(pg_tab), PTE_USER | PTE_PRESENT | PTE_WRITABLE
        );
    }

    // the last page_table might be half used
    {
        pte_t *pg_tab = boot_alloc(PAGE_SIZE, True);
        ASSERT(pg_tab != NULL);
        for (int j = 0; j <= __pte_idx(va); j++, pfn++) {
            pg_tab[j] = (pte_t)__pg_entry(
                pfn * PAGE_SIZE, PTE_USER | PTE_PRESENT | PTE_WRITABLE
            );
        }
        kern_pg_dir[__pde_idx(va)] = (pde_t)__pg_entry(
            __pa(pg_tab), PTE_USER | PTE_PRESENT | PTE_WRITABLE
        );
    }

    lcr3(__pa(kern_pg_dir));
}

// get the pate table entry for va
// @param pgidr the target page directory
// @param va the vitual address
// @param create when no such pte for va: if create is True, create one;
//               otherwise, does nothing
static pte_t *pgdir_walk(pde_t *pgdir, const void *va, bool_t create) {
    ASSERT(pgdir != NULL);
    uint32_t pde_idx = __pde_idx(va);
    pte_t *pg_tab = NULL;
    if (!(pgdir[pde_idx] & PDE_PRESENT)) {
        // pde is not present now
        if (!create) {
            return NULL;
        } else {
            ppage_t *fp = pages_alloc(1, GFP_ZERO);
            if (fp == NULL) {
                return NULL;
            }
            pg_tab = page2kva(fp);
            pgdir[pde_idx] = (pde_t)__pg_entry(
                __pa(pg_tab), PDE_PRESENT | PDE_WRITABLE | PDE_USER
            );
        }
    } else {
        pg_tab = __pte_kvaddr(pgdir[pde_idx]);
    }
    uint32_t pte_idx = __pte_idx(va);
    return &(pg_tab[pte_idx]);
}


void page_unmap(pde_t *pgdir, void *va) {
    pte_t *pte = pgdir_walk(pgdir, va, False);
    if (!(pte && (*pte & PTE_PRESENT))) {
        // if there is no mapped page, return
        return;
    }
    uintptr_t page_pa = *pte & PG_START_ADDRESS_MASK;
    ppage_t *p = pa2page(page_pa);
    page_decref(p);
    invlpg(va);
    *pte = 0;
}


int page_map(pde_t *pgdir, void *va, ppage_t *p, uint32_t perm) {
    pte_t *pte = pgdir_walk(pgdir, va, True);
    if (pte == NULL) {
        return ERR_MEMORY_SHORTAGE;
    }

    // panic if there's already a page mapped there
    ASSERT(!(*pte & PTE_PRESENT));

    *pte = (pte_t)__pg_entry(page2pa(p), PTE_PRESENT | perm);
    page_incref(p);
    return ERR_NO_ERR;
}


void page_dir_init(pde_t *pd) {
    for (int i = __pde_idx(KERNEL_BASE); i < NRPDE; i++) {
        pd[i] = kern_pg_dir[i];
    }
}


void load_page_dir(pde_t *pd) {
    if (pd == NULL) {
        lcr3(__pa(kern_pg_dir));
    } else {
        lcr3(__pa(pd));
    }
}
