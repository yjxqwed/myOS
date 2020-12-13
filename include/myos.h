#ifndef __MYOS_H__
#define __MYOS_H__

#include <arch/x86.h>
#include <common/types.h>

// Here are some information of myOS

// myOS requires at least 128 MiB physical memory
#define MIN_MEMORY_LIMIT_MB (128)

// myOS kernel use high 2G virtual address space
#define KERNEL_BASE (0x80000000)

// kernel space physical address
#define __pa(x) (uintptr_t)((uintptr_t)(x) - KERNEL_BASE)
// kernel space virtual address
#define __va(x) (uintptr_t)((uintptr_t)(x) + KERNEL_BASE)

#define __pte_kvaddr(pde) (pte_t *)__va((pde) & PG_START_ADDRESS_MASK)

#define __valid_kva(x) (((uintptr_t)(x) >= KERNEL_BASE) ? True : False)

enum {
    ERR_NO_ERR = 0,              // No err
    ERR_MEMORY_SHORTAGE,         // Operation failed due to memory shortage
};

#endif