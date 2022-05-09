#ifndef __MYOS_H__
#define __MYOS_H__

#include <arch/x86.h>
#include <common/types.h>

// Here are some information of myOS

// myOS requires at least 128 MiB physical memory
#define MIN_MEMORY_LIMIT_MB 128
#define MAX_MEMORY_LIMIT_MB 2048

// myOS kernel uses high 2G virtual address space
#define KERNEL_BASE 0x80000000

#define USER_STACK_BOTTOM (KERNEL_BASE - 4 * PAGE_SIZE)
// user stack size limit 16KiB
#define USER_STACK_LIMIT 0x00004000

#define USER_HEAP_BOTTOM 0x02000000
// user heap size limit 16MiB
#define USER_HEAP_LIMIT 0x01000000

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