#include <mm/pager.h>
#include <common/debug.h>
#include <sys/global.h>
#include <string.h>

pde_t *page_directory = (pde_t *)PD_BASE_ADDR;
uint32_t _pd = 0;  // for loader.s

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

extern void flushPD();

void init_pd() {
    memset(page_directory, 0, PAGE_SIZE);  // set pd to all 0s
    _pd = (uint32_t)page_directory;
    flushPD();
}