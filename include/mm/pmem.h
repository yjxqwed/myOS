#ifndef __PMEM_H__
#define __PMEM_H__

// physical memory management

#include <multiboot/multiboot.h>
#include <common/types.h>

// high mem starts at 1MiB
#define HIGH_MEM_BASE 0x00100000

typedef struct PhysicalPageInfo {
    // pointer to the next free ppage
    // if I'm free, next_free_ppage != NULL and vice versa
    struct PhysicalPageInfo *next_free_ppage;
    // num of references to this ppage
    // if num_ref == 0, I'm free
    uint32_t num_ref;
} ppage_t;

void *boot_alloc(uint32_t n, bool page_alligned);

// setup physical memory management system
void setup_memory(multiboot_info_t *mbi);

// The max page frame number of the machine
extern uint32_t max_high_pfn;

void pmem_init();
#endif