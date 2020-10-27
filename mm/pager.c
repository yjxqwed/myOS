#include <mm/pager.h>
#include <common/debug.h>
#include <sys/global.h>
#include <string.h>
#include <kprintf.h>
#include <bitmap.h>

#define NPDE (PAGE_SIZE / sizeof(pde_t))
#define NPTE (PAGE_SIZE / sizeof(pte_t))

pde_t *page_directory = (pde_t *)__pa(PD_BASE_ADDR);
uint32_t _pd = 0;  // for loader.s


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
        for (j = 0; j < NPTE; j++) {
            pte_t *pte = &(pt[j]);
            set_pte_ppage_number(pte, i * NPTE + j);
            set_pte_attr(pte, PTE_SUPERUSER, 1);
            set_pte_attr(pte, PTE_RW, 1);
            set_pte_attr(pte, PTE_PRESENT, 1);
        }
    }

    flushPD();
}




void clear_low_mem_mapping() {
    // TO FIGURE OUT: Does the low 1MiB memory need identity map?
    // (for bios/grub/hw use maybe)
    pde_t *pd = (pde_t *)__va(page_directory);
    for (int i = 0; i < 4; i++) {
        pd[i] = (pde_t)0;
    }
}
