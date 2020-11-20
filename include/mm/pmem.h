#ifndef __PMEM_H__
#define __PMEM_H__

/**
 *    Physical Memory Management
 */

#include <multiboot/multiboot.h>
#include <common/types.h>
#include <arch/x86.h>

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

// detect physical memory information
void detect_memory(multiboot_info_t *mbi);

void pmem_init();

// Get Free Page flags
#define GFP_ZERO 0x1

// alloc a physical page
ppage_t *page_alloc(uint32_t gfp_flags);

// free a page
// panic if p can not be freed
void page_free(ppage_t *p);

// get pg_cnt number of continuous free pages
// return the first page of the pg_cnt pages
ppage_t *pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags);

// free pg_cnt pages starting from p
void pages_free(ppage_t *p, uint32_t pg_cnt);

// increase the reference to p
void page_incref(ppage_t *p);

// decrease the reference to p, free it if no referrence
void page_decref(ppage_t *p);

// page to its kernel virtual address
void *page2kva(ppage_t *p);
// page to its physical address
void *page2pa(ppage_t *p);
// kernel virtual address to page
ppage_t *kva2page(void *kva);
// physical address to page
ppage_t *pa2page(void *pa);

/**
 *    Paging Management
 */

// install the pre-mapped pd and pt for bootstrap
void install_boot_pg(void);

// init the kernel's static mappings
void kernel_init_paging(void);

// unmap the page mapped and including va; 
// if no page mapped, do nothing
// kernel won't use this
//     @param pgdir page directory
//     @param va virtual address
void page_unmap(pde_t *pgdir, void *va);

// map the page p at virtual address va; 
// panic if there's already a page mapped at va
//     @param pgdir page directory
//     @param va virtual address, should be page alligned
//     @param p the physical page
int page_map(pde_t *pgdir, void *va, ppage_t *p, uint32_t perm);
#endif