#ifndef __VMM_H__
#define __VMM_H__

#include <mm/pmem.h>
#include <common/types.h>

void vmm_init();

// @brief get pgcnt pages of continuous memory,
// for kernel use only
// @param pgcnt number of pages requested
// @return kva on success or NULL on failure
void *k_get_free_pages(uint32_t pgcnt, uint32_t gfp_flags);

// @brief free pgcnt pages of continuous memory starting from va,
// for kernel use only
// @param kva kernel virtual address, page alligned
// @param pgcnt number of pages to be freed
void k_free_pages(void *kva, uint32_t pgcnt);


// there are 7 kinds of blocks 16B each -> 1024B each
#define MIN_BLK_SIZE    32
#define MAX_BLK_SIZE    1024
#define NR_MEM_BLK_DESC 6

// @brief get bytes of memory. FOR KERNEL USE ONLY!
// @param size number of bytes
// @return kva of the allocated block
void *kmalloc(uint32_t size);

// @brief free the memory block allocated by kmalloc, 
// panic otherwise. FOR KERNEL USE ONLY!
// @param va the address allocated by kmalloc
void kfree(void *va);

void vmm_print();

#endif