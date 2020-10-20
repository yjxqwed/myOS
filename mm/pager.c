#include <mm/pager.h>
#include <common/debug.h>
#include <sys/global.h>
#include <string.h>
#include <kprintf.h>

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


#define CHECK_FLAG(flags,bit) ((flags) & (1 << (bit)))
void print_mem_info(multiboot_info_t *mbi) {
    kprintf(KPL_NOTICE, "================ System Info ================\n");
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
        multiboot_memory_map_t *mmap;
        kprintf(
            KPL_NOTICE, "mmap_addr = 0x%X, mmap_length = 0x%X\n",
            (uint32_t)mbi->mmap_addr, (uint32_t)mbi->mmap_length
        );
        for (
            mmap = (multiboot_memory_map_t *)mbi->mmap_addr;
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
                (uint32_t) mmap->type)
            ;
        }
    }
    kprintf(KPL_NOTICE, "=============================================\n");
}