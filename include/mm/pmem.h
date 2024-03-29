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
#include <mm/mm_info.h>

// high mem starts at 1MiB
#define HIGH_MEM_BASE 0x00100000

typedef struct PhysicalPageInfo ppage_t;

struct PhysicalPageInfo {
    // num of references to this ppage
    // if num_ref == 0, I'm free
    uint32_t num_ref;

    // bool_t free;
    // every operation of this page struct should acquire this lock
    mutex_t page_lock;
};

// detect physical memory information
void detect_memory(multiboot_info_t *mbi);
// init the pmem management structures
void pmem_init();

// Get Free Page flags
#define GFP_ZERO 0x1


/**
 *  After getting page(s) from pages_alloc, incref it(them).
 *  When you don't need it(them), decref it(them).
 */

/**
 * @brief get pg_cnt number of continuous free physical pages
 * 
 * @param pg_cnt number of free physical pages needed
 * @param gfp_flags get_free_page flags
 * @return ppage_t* pointer to the first allocated page;
 *         NULL if failed to allocate
 */
ppage_t *pages_alloc(uint32_t pg_cnt, uint32_t gfp_flags);

/**
 * @brief free pg_cnt physical pages
 * 
 * @param ps array of pointers to ppage_t
 * @param pg_cnt length of ps
 */
void pages_free(ppage_t **ps, uint32_t pg_cnt);

/**
 * @brief Increase the reference to p; panic if p is free
 * 
 * @param p the physical page to incref
 */
void page_incref(ppage_t *p);

/**
 * @brief Decrease the reference to p, free p if no referrence;
 *        panic if p is free
 * 
 * @param p the physical page to decref
 */
void page_decref(ppage_t *p);


// get the static zero-out-ed page
ppage_t *get_zpage();
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

// install the pre-mapped pd and pt for bootstrap;
// enable paging
void install_boot_pg(void);

// init the kernel's static mappings
void kernel_init_paging(void);

// @brief unmap the page mapped and including va;
// if no page mapped, do nothing.
// kernel won't use this
//     @param pgdir page directory
//     @param va virtual address
void page_unmap(pde_t *pgdir, void *va);

// @brief map the page p at virtual address va;
// panic if there's already a page mapped at va
//     @param pgdir page directory
//     @param va virtual address, should be page alligned
//     @param p the physical page
int page_map(pde_t *pgdir, void *va, ppage_t *p, uint32_t perm);

// @brief init the page directory, will copy the kernel space
//        to it.
// @param pd the page direcotry to be initialized.
void page_dir_init(pde_t *pd);

// @brief load pd to cr3 if pd is not NULL,
//        else load kern_pg_dir
void load_page_dir(pde_t *pd);


/**
 *    For debug only
 */


void print_page(ppage_t *p);
void pmem_print();

int sys_mm(mm_info_t *mm_info);

#endif