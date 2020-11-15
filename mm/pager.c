#include <mm/pager.h>
#include <common/debug.h>
#include <string.h>
#include <kprintf.h>
#include <bitmap.h>
// #include <mm/mem.h>
#include <mm/pmem.h>
#include <arch/x86.h>
#include <myos.h>

// pde_t *page_directory = (pde_t *)__pa(PD_BASE_ADDR);
// uintptr_t _pd = 0;  // for loader.s

#define __PAGE_ALLIGNED __attribute__((aligned(PAGE_SIZE)))

// For bootstrap paging use
__PAGE_ALLIGNED pde_t boot_pg_dir[NRPDE];
__PAGE_ALLIGNED pte_t boot_pg_tab[NRPTE];

void install_boot_pg(void) {
    pte_t *ppg = (pte_t *)__pa(boot_pg_tab);
    for (int i = 0; i < NRPTE; i++) {
        ppg[i] = (pte_t)__pg_entry(i, PTE_PRESENT | PTE_WRITABLE);
    }
    memset(boot_pg_dir, 0, PAGE_SIZE);
    pde_t pde = (pde_t)__pg_entry(
        __page_number(__pa(boot_pg_tab)), PTE_PRESENT | PTE_WRITABLE
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
                pfn, PTE_USER | PTE_PRESENT
            );
        }
        kern_pg_dir[i] = (pde_t)__pg_entry(
            __page_number(__pa(pg_tab)), PTE_USER | PTE_PRESENT
        );
    }

    // the last page_table might be half used
    do {
        pte_t *pg_tab = boot_alloc(PAGE_SIZE, true);
        ASSERT(pg_tab != NULL);
        for (int j = 0; j <= __pte_idx(va); j++, pfn++) {
            pg_tab[j] = (pte_t)__pg_entry(
                pfn, PTE_USER | PTE_PRESENT
            );
        }
        kern_pg_dir[__pde_idx(va)] = (pde_t)__pg_entry(
            __page_number(__pa(pg_tab)), PTE_USER | PTE_PRESENT
        );
    } while (0);

    lcr3(__pa(kern_pg_dir));
    kprintf(KPL_DUMP, "0x%X\n", boot_alloc(0, true));
    kprintf(KPL_DUMP, "0x%X\n", boot_alloc(0, true));
}