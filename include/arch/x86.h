#ifndef __X86_H__
#define __X86_H__

/**************************************
 * 
 * This file contains the x86 platform
 *  related macros.
 * 
***************************************/

#include <common/types.h>

// =================== PAGING/MMU =====================

// page size = 4kb
#define PAGE_SIZE 0x1000

// page directory shift -- the high 10 bits is page directory idx
#define PD_IDX_SHIFT 22
// page table shift -- the mid 10 bit2 is page table idx
#define PT_IDX_SHIFT 12

// x86 use 10-bit index for pde and pte
#define PD_IDX_MASK (0x3ff << PD_IDX_SHIFT)
#define PT_IDX_MASK (0x3ff << PT_IDX_SHIFT)

#define PG_START_ADDRESS_MASK 0xfffff000

#define __pg_start_addr(x) ((x) & PG_START_ADDRESS_MASK)

#define __pde_idx(x) ((x) >> PD_IDX_SHIFT)
#define __pte_idx(x) (((x) & PT_IDX_MASK) >> PT_IDX_SHIFT)

#define __pg_entry(ppage_no, attr) (uint32_t)(((ppage_no) * PAGE_SIZE) | (attr))

#define __page_number(x) (uint32_t)(((uintptr_t)(x)) / PAGE_SIZE)

// ========== Page Directory ==========

typedef uint32_t PageDirectoryEntry;
typedef PageDirectoryEntry pde_t;

// ignored
#define PDE_GLOBAL        0x100
// 0 = 4KiB, 1 = 4MiB (PSE needed)
// should be 0
#define PDE_PAGE_SIZE     0x080
// accessed (set by CPU)
#define PDE_ACCESSED      0x020
// set to disable cache
#define PDE_CACHE_DISABLE 0x010
// set to enable write-through caching
// clear to enable write-back caching
#define PDE_WRITE_THRU    0x008
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PDE_USER          0x004
// set to make it writable; clear to make it read-only
#define PDE_WRITABLE      0x002
// set = in mem; clear = swapped-out
#define PDE_PRESENT       0x001

// void set_pde_attr(pde_t *pde, int attr, int val);

// ========== Page Table ==========
typedef uint32_t PageTableEntry;
typedef PageTableEntry pte_t;

// set to prevent the TLB from updating the
// address in its cache if cr3 is reset
// (page global enable bit in cr4 must also be set)
#define PTE_GLOBAL        0x100
// unused (too complex)
#define PTE_PAGE_TABLE_ATTR_IDX 0x080
// set = this page is written to (set by cpu)
#define PTE_DIRTY         0x040
// set = accessed (set by cpu)
#define PTE_ACCESSED      0x020
// set to disable cache
#define PTE_CACHE_DISABLE 0x010
// set to enable write-through caching
// clear to enable write-back caching
#define PTE_WRITE_THRU    0x008
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PTE_USER          0x004
// set to make it writable; clear to make it read-only
#define PTE_WRITABLE      0x002
// set = in mem; clear = swapped-out
#define PTE_PRESENT       0x001

// number of pdes per page directory
#define NRPDE 1024
// number of ptes per page table
#define NRPTE 1024

// amount of memory controlled by a pg_tab (4 MiB)
#define MEM_SIZE_PER_PGTABLE (NRPTE * PAGE_SIZE)

// ===================== End of MMU ===========================


#endif