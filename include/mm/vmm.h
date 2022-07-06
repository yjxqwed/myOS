#ifndef __VMM_H__
#define __VMM_H__

typedef struct vmm_struct vmm_t;

#include <arch/x86.h>
#include <common/types.h>

/**
 * @brief struct related to process address space
 */
struct vmm_struct {

    // every process has its own page directory
    pde_t *pgdir;

    uint32_t args_end;
    uint32_t args_start;

    uint32_t stack_bot;
    uint32_t stack_top;

    uint32_t map_bot;
    uint32_t map_top;

    // real_heap_top: the space that is mapped to pages
    uint32_t real_heap_top;
    uint32_t heap_top;
    uint32_t heap_bot;

    uint32_t data_end;
    uint32_t data_start;

    uint32_t code_end;
    uint32_t code_start;

};

/**
 * @brief Initialize a vmm object
 */
int init_vmm_struct(vmm_t *vmm);

/**
 * @brief Destroy a vmm object
 */
void destroy_vmm_struct(vmm_t *vmm);

void *sys_brk(uintptr_t __addr);

/**
 * @brief map <np> pages to the space starting with va
 * 
 * @return 0 if succeed
 */
int vmm_map_pages(pde_t *pd, uint32_t va, size_t np, uint32_t perm);

#endif