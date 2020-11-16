#ifndef __PMEM_H__
#define __PMEM_H__

/**
 *    Physical Memory Management
 */

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

// setup physical memory management system
void setup_memory(multiboot_info_t *mbi);

// The max page frame number of the machine
extern uint32_t max_high_pfn;

void pmem_init();


// Get Free Page flags
#define GFP_ZERO 0x1

// alloc a physical page
ppage_t *page_alloc(uint32_t gfp_flags);
// alloc a physical page and get its kernel virtual address
// for kernel use only
void *kv_get_page(uint32_t gfp_flags);
// free a page
void page_free(ppage_t *p);
// decrease the reference to p, free it if no more referrence
void page_decref(ppage_t *p);

/**
 *    Paging Management
 */

void install_boot_pg(void);
void kernel_init_paging(void);
#endif