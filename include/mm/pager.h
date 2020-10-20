#ifndef __PAGER_H__
#define __PAGER_H__

#include <common/types.h>
#include <multiboot/multiboot.h>

// For Page Directory and Page Table

// page size = 4kb
#define PAGE_SIZE 0x1000

// ========== Page Directory ==========

typedef uint32_t PageDirectoryEntry;
typedef PageDirectoryEntry pde_t;

// addr is 4KB aligned
void set_pde_addr(pde_t *pde, uint32_t addr);

// ignored
#define PDE_GLOBAL        8
// 0 = 4KiB, 1 = 4MiB (PSE needed)
// should be 0
#define PDE_PAGE_SIZE     7
// accessed (set by CPU)
#define PDE_ACCESSED      5
// set to disable cache
#define PDE_CACHE_DISABLE 4
// set to enable write-through caching
// clear to enable write-back caching
#define PDE_WRITE_THRU    3
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PDE_SUPERUSER     2
// set to make it writable; clear to make it read-only
#define PDE_RW            1
// set = in mem; clear = swapped-out
#define PDE_PRESENT       0

void set_pde_attr(pde_t *pde, int attr, int val);

// ========== Page Table ==========
typedef uint32_t PageTableEntry;
typedef PageTableEntry pte_t;

// addr is 4KB aligned
void set_pde_addr(pte_t *pte, uint32_t addr);

// set to prevent the TLB from updating the
// address in its cache if cr3 is reset
// (page global enable bit in cr4 must also be set)
#define PTE_GLOBAL        8
// unused (too complex)
#define PTE_PAGE_TABLE_ATTR_IDX 7
// set = this page is written to (set by cpu)
#define PTE_DIRTY         6
// set = accessed (set by cpu)
#define PTE_ACCESSED      5
// set to disable cache
#define PTE_CACHE_DISABLE 4
// set to enable write-through caching
// clear to enable write-back caching
#define PTE_WRITE_THRU    3
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PTE_SUPERUSER     2
// set to make it writable; clear to make it read-only
#define PTE_RW            1
// set = in mem; clear = swapped-out
#define PTE_PRESENT       0

void set_pde_attr(pte_t *pte, int attr, int val);

// enable and init page directory
void init_pd();
void print_mem_info(multiboot_info_t *mbi);
#endif