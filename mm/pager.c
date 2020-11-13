#include <mm/pager.h>
#include <common/debug.h>
#include <string.h>
#include <kprintf.h>
#include <bitmap.h>
#include <mm/mem.h>
#include <arch/x86.h>
#include <myos.h>

// pde_t *page_directory = (pde_t *)__pa(PD_BASE_ADDR);
uintptr_t _pd = 0;  // for loader.s

#define __PAGE_ALLIGNED __attribute__((aligned(PAGE_SIZE)))

__PAGE_ALLIGNED pde_t boot_pg_dir[NRPDE];
__PAGE_ALLIGNED pte_t boot_pg_tab[NRPTE];


uint32_t _boot_pd = __pa(boot_pg_dir);
void install_boot_pg(void) {
    pte_t *ppg = (pte_t *)__pa(boot_pg_tab);
    for (int i = 0; i < NRPTE; i++) {
        // MAGICBP;
        ppg[i] = (pte_t)__pg_entry(i, PTE_PRESENT | PTE_WRITABLE);
        // kprintf("boot_pg_tab[%d]=0x%X\n", i, boot_pg_tab[i]);
    }
    memset(boot_pg_dir, 0, PAGE_SIZE);
    pde_t pde = (pde_t)__pg_entry(
        __page_number(__pa(boot_pg_tab)), PTE_PRESENT | PTE_WRITABLE
    );
    // kprintf(KPL_DEBUG, "pde=0x%X\n", pde);
    pde_t *ppd = (pde_t *)__pa(boot_pg_dir);
    ppd[0] = pde;
    ppd[__pde_idx(KERNEL_BASE)] = pde;
    extern void flush_boot_pd();
    // kprintf(KPL_DEBUG, "boot_pg_dir=0x%X, &boot_pg_dir=0x%X, _boot_pd=0x%X\n", boot_pg_dir, &boot_pg_dir, _boot_pd);
    // kprintf(KPL_DEBUG, "pd[0]=0x%X\n", boot_pg_dir[0]);
    // kprintf(KPL_DEBUG, "pd[0x%x]=0x%X\n", __pde_idx(KERNEL_BASE), boot_pg_dir[__pde_idx(KERNEL_BASE)]);
    flush_boot_pd();
}


extern void flushPD();

// enable paging, init pd and pts.
void init_paging() {
    // memset((uint8_t *)page_directory, 0, PAGE_SIZE);  // set pd to all 0s
    // _pd = (uint32_t)page_directory;
    // // the first pt starts at after the pd.
    // uint32_t first_pt_ppage_no = __pa(PD_BASE_ADDR) / PAGE_SIZE + 1;

    // int i, j;
    // // pdes 0x300 ~ 0x3fe are for vaddr 0xc0000000 ~ 0xffbfffff (high 1G - 4MiB)
    // for (i = 0x300, j = 0; i < 0x400; i++, j++) {
    //     pde_t *pde = &(page_directory[i]);
    //     set_pde_ppage_number(pde, first_pt_ppage_no + j);
    //     set_pde_attr(pde, PDE_SUPERUSER, 1);
    //     set_pde_attr(pde, PDE_RW, 1);
    //     set_pde_attr(pde, PDE_PRESENT, 1);
    // }
    // // the last pde refers the PD itself.
    // set_pde_ppage_number(&(page_directory[0x3ff]), first_pt_ppage_no);

    // // pdes 0x0 ~ 0x8 are for vaddr 0x0 ~ 0x01ffffff
    // // for cpu to move to paging mode
    // for (i = 0, j = 0; i < 8; i++, j++) {
    //     pde_t *pde = &(page_directory[i]);
    //     set_pde_ppage_number(pde, first_pt_ppage_no + j);
    //     set_pde_attr(pde, PDE_SUPERUSER, 1);
    //     set_pde_attr(pde, PDE_RW, 1);
    //     set_pde_attr(pde, PDE_PRESENT, 1);
    // }

    // // init the first 8 page tables of the kernel space;
    // // 0xc0000000 ~ 0xc1ffffff -> 0x00000000 ~ 0x01ffffff
    // int num_reserved_pages = FREE_MEM_BASE / MEM_SIZE_PER_PGTABLE;
    // for (i = 0; i < num_reserved_pages; i++) {
    //     pte_t *pt = (pte_t *)(
    //         page_directory[0x300 + i] >> 12 << 12
    //     );
    //     memset((uint8_t *)pt, 0, PAGE_SIZE);
    //     // each page table has 1024 entries
    //     for (j = 0; j < NPTE; j++) {
    //         pte_t *pte = &(pt[j]);
    //         set_pte_ppage_number(pte, i * NPTE + j);
    //         set_pte_attr(pte, PTE_SUPERUSER, 1);
    //         set_pte_attr(pte, PTE_RW, 1);
    //         set_pte_attr(pte, PTE_PRESENT, 1);
    //     }
    // }

    // flushPD();
}


void clear_low_mem_mapping() {
    // TO FIGURE OUT: Does the low 1MiB memory need identity map?
    // (for bios/grub/hw use maybe)
    // pde_t *pd = (pde_t *)__va(page_directory);
    // for (int i = 0; i < 8; i++) {
    //     pd[i] = (pde_t)0;
    // }
}
