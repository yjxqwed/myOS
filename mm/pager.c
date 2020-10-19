#include <mm/pager.h>
#include <common/debug.h>

pde_t *page_directory = (pde_t *)0x10000;

void set_pde_addr(pde_t *pde, uint32_t addr) {
    *pde = (addr << 12) | (*pde & 0xfff);
}

void set_pte_addr(pte_t *pte, uint32_t addr) {
    *pte = (addr << 12) | (*pte & 0xfff);
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