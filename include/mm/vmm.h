#ifndef __VMM_H__
#define __VMM_H__

typedef struct vmm_struct vmm_t;

#include <arch/x86.h>
#include <common/types.h>
#include <thread/sync.h>


/**
 * @brief struct related to process address space
 */
struct vmm_struct {

    // every process has its own page directory
    pde_t *pgdir;
    mutex_t *vmm_mutex;

    uint32_t args_end;
    uint32_t args_start;

    uint32_t stack_bot;
    uint32_t stack_top;

    uint32_t map_bot;
    uint32_t map_top;

    uint32_t heap_top;
    uint32_t heap_bot;

    uint32_t data_end;
    uint32_t data_start;

    uint32_t code_end;
    uint32_t code_start;

};

/**
 * @brief Initialize a vmm object
 * 
 * @param vmm The object to initialize
 * @return bool_t True on success and False on failure
 */
bool_t init_vmm_struct(vmm_t *vmm);

/**
 * @brief Destroy a vmm object
 * 
 * @param vmm The object to destroy
 */
void destroy_vmm_struct(vmm_t *vmm);

#endif