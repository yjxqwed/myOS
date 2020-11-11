#ifndef __MEM_H__
#define __MEM_H__

#include <multiboot/multiboot.h>
#include <mm/pager.h>
#include <bitmap.h>
#include <common/types.h>
#include <sys/global.h>

// myOS requires at least 254 MiB free mem
// 254 = 256 - low 1 MiB - high 1 MiB
#define MEM_LIMIT 254 * 1024 * 1024

// myOS never uses low 1 MiB memory
#define AVAIL_MEM_BASE 0x00100000
#define AVAIL_PAGE_BASE (AVAIL_MEM_BASE / PAGE_SIZE)

// myOS reserves high 1 MiB ~ 31 MiB mem
#define K_RESERVED_MEM 0x01F00000

// 1. mem below AVAIL_MEM_BASE is reserved for BIOS and GRUB
// 2. myOS reserves K_RESERVED_MEM amount of mem for system data structures use
//    (direct mapping)
// 3. mem above FREE_MEM_BASE is free to use for allocating
#define FREE_MEM_BASE (AVAIL_MEM_BASE + K_RESERVED_MEM)

// myOS kernel only uses up to 512 MiB physical mem
#define K_NEEDED_MEM 0x20000000

// myOS kernel doesn't have normal zone
#define K_NORMAL_ZONE_SIZE 0

// myOS kernel allocate virtual pages from 0xd0000000
#define K_VM_BASE_ADDR (KERNEL_SPACE_BASE_ADDR + FREE_MEM_BASE + K_NORMAL_ZONE_SIZE)

// start (virtual) address of the kernel heap
#define KERNEL_HEAP_BASE_ADDR (KERNEL_SPACE_BASE_ADDR + FREE_MEM_BASE)

void print_mem_info(multiboot_info_t *mbi);

// memory management init
void mm_init(multiboot_info_t *mbi);

// get pg_cnt number pages of memory in the kernel virtual space
//     the physical pages is not guaranteed to be continuous
// reutrn the starting address; NULL if the request is not satisfied 
//     (not enough virtual space or no physical pages)
void *vm_kernel_get_pages(uint32_t pg_cnt);

void vm_kernel_free_pages();

typedef enum PHY_POOL_FLAG {
    PPF_KERNEL,
    PPF_USER,
    PPF_SIZE
} PHY_POOL_FLAG;

// the memory page pool is page (4 KiB) granular and a continuous region
// @members:
//   btmp is the bitmap for allocating pages
//   num_total_pages is the total amount of pages in this pool
//   start_page_number is the starting page's index of this pool
typedef struct MemoryPagePool {
    btmp_t btmp;
    uint32_t num_total_pages;
    uint32_t start_page_number;
} mpp_t;

// a virtual memory area
// @members:
//   btmp is the btmp
//   vm_start is the start address of the area
//   vm_end is the end address of the area
//   - vma should be page aligned
typedef struct VirtualMemoryArea {
    btmp_t btmp;
    uint32_t vm_start;
    uint32_t vm_end;
} vma_t;

// get a physical page from a physical page pool
// return the (physical) base address of the page
//        NULL otherwise
void *get_ppage(mpp_t *pool);

// get pg_cnt number pages of memory in the kernel virtual space
//     the physical pages is not guaranteed to be continuous
// reutrn the starting address; NULL if the request is not satisfied 
//     (not enough virtual space or no physical pages)
// the pages are get above K_VM_BASE_ADDR
// - Get 256 pages at most per request
void *kernel_vmalloc(uint32_t pg_cnt);

// get a region from vma with size of pg_cnt pages
void *get_vaddr(vma_t *vma, uint32_t pg_cnt);

#endif