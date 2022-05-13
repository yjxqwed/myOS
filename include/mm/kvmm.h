#ifndef __KVMM_H__
#define __KVMM_H__

#include <mm/pmem.h>
#include <common/types.h>

void vmm_init();

/**
 * @brief get pgcnt pages of continuous memory. FOR KERNEL USE ONLY!
 * 
 * @param pgcnt number of pages requested
 * @param gfp_flags get_free_page flags
 * @return void* kva of the first page allocated; NULL on failure
 */
void *k_get_free_pages(uint32_t pgcnt, uint32_t gfp_flags);

/**
 * @brief free pgcnt pages allocated by k_get_free_pages(). FOR KERNEL USE ONLY!
 * 
 * @param kva kernel virtual address, page alligned
 * @param pgcnt number of pages to be freed
 */
void k_free_pages(void *kva, uint32_t pgcnt);

/**
 * @brief get at least size bytes of zero-outed memory. FOR KERNEL USE ONLY!
 * 
 * @param size number of bytes
 * @return void* kva of the allocated memory; Null on failure
 */
void *kmalloc(uint32_t size);

/**
 * @brief free the memory block allocated by kmalloc. FOR KERNEL USE ONLY!
 * 
 * @param kva the address returned by kmalloc
 */
void kfree(void *kva);

// for debug only
void vmm_print();

#endif