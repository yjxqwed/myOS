#include <mm/pager.h>
#include <common/debug.h>
#include <sys/global.h>
#include <string.h>
#include <kprintf.h>
#include <bitmap.h>

pde_t *page_directory = (pde_t *)__pa(PD_BASE_ADDR);
uint32_t _pd = 0;  // for loader.s


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


// both pde and pte hold physical page number
// otherwise a recursive lookup will occur
void set_pde_ppage_number(pde_t *pde, uint32_t phy_page_number) {
    *pde = (phy_page_number << 12) | (*pde & 0xfff);
}

void set_pte_ppage_number(pte_t *pte, uint32_t phy_page_number) {
    *pte = (phy_page_number << 12) | (*pte & 0xfff);
}

void set_pde_attr(pde_t *pde, int attr, int val) {
    if (val) {
        *pde |= (1 << attr);
    } else {
        *pde &= ~(1 << attr);
    }
}

void set_pte_attr(pde_t *pte, int attr, int val) {
    if (val) {
        *pte |= (1 << attr);
    } else {
        *pte &= ~(1 << attr);
    }
}

extern void flushPD();

// enable paging, init pd and pts.
void init_paging() {
    memset((uint8_t *)page_directory, 0, PAGE_SIZE);  // set pd to all 0s
    _pd = (uint32_t)page_directory;
    // the first pt starts at after the pd.
    uint32_t first_pt_ppage_no = __pa(PD_BASE_ADDR) / PAGE_SIZE + 1;

    int i, j;
    // pdes 0x300 ~ 0x3fe are for vaddr 0xc0000000 ~ 0xffbfffff (high 1G - 4MiB)
    for (i = 0x300, j = 0; i < 0x400; i++, j++) {
        pde_t *pde = &(page_directory[i]);
        set_pde_ppage_number(pde, first_pt_ppage_no + j);
        set_pde_attr(pde, PDE_SUPERUSER, 1);
        set_pde_attr(pde, PDE_RW, 1);
        set_pde_attr(pde, PDE_PRESENT, 1);
    }
    // the last pde refers the PD itself.
    set_pde_ppage_number(&(page_directory[0x3ff]), first_pt_ppage_no);

    // pdes 0x0 ~ 0x3 are for vaddr 0x0 ~ 0x00bfffff
    // for cpu to move to paging mode
    for (i = 0, j = 0; i < 4; i++, j++) {
        pde_t *pde = &(page_directory[i]);
        set_pde_ppage_number(pde, first_pt_ppage_no + j);
        set_pde_attr(pde, PDE_SUPERUSER, 1);
        set_pde_attr(pde, PDE_RW, 1);
        set_pde_attr(pde, PDE_PRESENT, 1);
    }

    // init the first 4 page tables of the kernel space;
    // 0xc0000000 ~ 0xc1000000 -> 0x00000000 ~ 0x01000000
    for (i = 0; i < 4; i++) {
        pte_t *pt = (pte_t *)((page_directory[0x300 + i] >> 12) << 12);
        memset((uint8_t *)pt, 0, PAGE_SIZE);
        // each page table has 1024 entries
        for (j = 0; j < 1024; j++) {
            pte_t *pte = &(pt[j]);
            set_pte_ppage_number(pte, i * 1024 + j);
            set_pte_attr(pte, PTE_SUPERUSER, 1);
            set_pte_attr(pte, PTE_RW, 1);
            set_pte_attr(pte, PTE_PRESENT, 1);
        }
    }

    flushPD();
}


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

void clear_low_mem_mapping() {
    // TO FIGURE OUT: Does the low 1MiB memory need identity map?
    // (for bios/grub/hw use maybe)
    pde_t *pd = (pde_t *)__va(page_directory);
    for (int i = 0; i < 4; i++) {
        pd[i] = (pde_t)0;
    }
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
        uint32_t high_free_pages = high_free_mem / 4096;
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