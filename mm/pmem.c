#include <mm/pmem.h>
#include <bitmap.h>
#include <kprintf.h>
#include <common/types.h>
#include <common/debug.h>
#include <string.h>
#include <myos.h>
#include <arch/x86.h>

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
    kprintf(KPL_NOTICE, "=============================================\n");
}

void detect_memory(multiboot_info_t *mbi) {
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
        extern void *kernel_image_end;
        // _end is the (physical) end of the kernel binary image
        uintptr_t _end = __pa(&kernel_image_end);
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
// On success, return the kernel virtual address; NULL otherwise.
// @param n number of bytes required
// @param page_alligned whether the block should be page alligned
static void *boot_alloc(uint32_t n, bool page_alligned) {

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

// page to kernel virtual address
void *page2kva(ppage_t *p) {
    return (void *)((p - pmap) * PAGE_SIZE);
}

// page to physical address
void *page2pa(ppage_t *p) {
    return __pa(page2kva(p));
}

ppage_t *kva2page(void *kva) {
    return pmap + (uintptr_t)kva / PAGE_SIZE;
}

ppage_t *pa2page(void *pa) {
    return kva2page(__va(pa));
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

// alloc a physical page
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

void page_free(ppage_t *p) {
    ASSERT(p != NULL);
    ASSERT(p->num_ref == 0);
    ASSERT(p->next_free_ppage == NULL);
    p->next_free_ppage = free_pages_list;
    free_pages_list = p;
}


void page_decref(ppage_t *p) {
    ASSERT(p != NULL);
    ASSERT(p->num_ref > 0);
    ASSERT(p->next_free_ppage == NULL);
    p->num_ref--;
    if (p->num_ref == 0) {
        page_free(p);
    }
}


/**
 *  Paging Management
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


// the page directory used by kernel
static pte_t *kern_pg_dir = NULL;

void kernel_init_paging() {
    kern_pg_dir = boot_alloc(PAGE_SIZE, true);
    kprintf(KPL_DEBUG, "kern_pg_dir=0x%X\n", (uintptr_t)kern_pg_dir);
    int pfn = 0;
    uintptr_t va = __va(max_high_pfn * PAGE_SIZE);
    kprintf(KPL_DEBUG, "va=0x%X\n", va);
    for (
        int i = __pde_idx(KERNEL_BASE);
        i < __pde_idx(va);
        i++
    ) {
        pte_t *pg_tab = boot_alloc(PAGE_SIZE, true);
        ASSERT(pg_tab != NULL);
        for (int j = 0; j < NRPTE; j++, pfn++) {
            pg_tab[j] = (pte_t)__pg_entry(
                pfn * PAGE_SIZE, PTE_USER | PTE_PRESENT
            );
        }
        kern_pg_dir[i] = (pde_t)__pg_entry(
            __pa(pg_tab), PTE_USER | PTE_PRESENT
        );
    }

    // the last page_table might be half used
    {
        pte_t *pg_tab = boot_alloc(PAGE_SIZE, true);
        ASSERT(pg_tab != NULL);
        for (int j = 0; j <= __pte_idx(va); j++, pfn++) {
            pg_tab[j] = (pte_t)__pg_entry(
                pfn * PAGE_SIZE, PTE_USER | PTE_PRESENT
            );
        }
        kern_pg_dir[__pde_idx(va)] = (pde_t)__pg_entry(
            __pa(pg_tab), PTE_USER | PTE_PRESENT
        );
    }

    lcr3(__pa(kern_pg_dir));
}

// get the pate table entry for va
// @param pgidr the target page directory
// @param va the vitual address
// @param create when no such pte for va: if create is true, create one;
//               otherwise, does nothing
static pte_t *pgdir_walk(pde_t *pgdir, const void *va, bool create) {
    ASSERT(pgdir != NULL);
    uint32_t pde_idx = __pde_idx(va);
    pte_t *pg_tab = NULL;
    if (!(pgdir[pde_idx] & PDE_PRESENT)) {
        // pde is not present now
        if (!create) {
            return NULL;
        } else {
            ppage_t *fp = page_alloc(GFP_ZERO);
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
    pte_t *pte = pgdir_walk(pgdir, va, false);
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
    pte_t *pte = pgdir_walk(pgdir, va, true);
    if (pte == NULL) {
        return ERR_MEMORY_SHORTAGE;
    }

    // panic if there's already a page mapped there
    ASSERT(!(*pte & PTE_PRESENT));

    *pte = (pte_t)__pg_entry(page2pa(p), PTE_PRESENT | perm);

    return ERR_NO_ERR;
}