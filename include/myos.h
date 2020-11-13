#ifndef __MYOS_H__
#define __MYOS_H__

// Here are some information of myOS

// myOS requires at least 128 MiB physical memory
#define MIN_MEMORY_LIMIT (126 * 1024 * 1024)

// myOS kernel use high 1G virtual address space
#define KERNEL_BASE (0xc0000000)

// kernel space physical address
#define __pa(x) (uintptr_t)((uintptr_t)(x) - KERNEL_BASE)
// kernel space virtual address
#define __va(x) (uintptr_t)((uintptr_t)(x) + KERNEL_BASE)

#endif