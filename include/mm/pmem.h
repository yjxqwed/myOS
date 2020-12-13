#ifndef __PMEM_H__
#define __PMEM_H__

/**
 *    Physical Memory Management
 */

#include <multiboot/multiboot.h>
#include <common/types.h>
#include <arch/x86.h>
#include <list.h>
#include <thread/sync.h>

// high mem starts at 1MiB
#define HIGH_MEM_BASE 0x00100000

typedef struct PhysicalPageInfo ppage_t;

struct PhysicalPageInfo {
    // ppage_t *next_free;
    // ppage_t *prev_free;

    // doubly linked list in the free pages list
    list_node_t free_list_tag;
    // num of references to this ppage
    // if num_ref == 0, I'm free
    uint32_t num_ref;
    bool_t free;
    // every operation of this page struct should
    // acquire this lock
    mutex_t page_lock;
};

// detect physical memory information
void detect_memory(multiboot_info_t *mbi);

void pmem_init();

// Get Free Page flags
#define GFP_ZERO 0x1


/**
 *  After getting page(s) from pages_alloc, incref it(them).
 *  When you don't need it(them), decref it(them). There is
 *  no free.
 */


// get pg_cnt number of continuous free pages
// return the first page of the pg_cnt pages
ppage_t *pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags);

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

void pmem_print();

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