#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <common/types.h>

// file for some global macros

// vaddr 0xc0000000 - 0xffffffff is kernel space
#define KERNEL_SPACE_BASE_ADDR (0xc0000000)

// kernel physical memory pool bitmap base addr
#define KPPOOL_BTMP_BASE_ADDR (0x00700b00 + KERNEL_SPACE_BASE_ADDR)
// user physical memory pool bitmap base addr
#define UPPOOL_BTMP_BASE_ADDR (0x00708b00 + KERNEL_SPACE_BASE_ADDR)
// kernel virtual memory pool bitmap base addr
#define KVPOOL_BTMP_BASE_ADDR (0x00720b00 + KERNEL_SPACE_BASE_ADDR)

// page directory base address
#define PD_BASE_ADDR   (0x00800000 + KERNEL_SPACE_BASE_ADDR)

// video memory base address
#define VIDEO_MEM      (0x000B8000 + KERNEL_SPACE_BASE_ADDR)

// kernel stack is 4 MiB starting from 0x00cf ffff
#define K_STACK_TOP    (0x01ffffff + KERNEL_SPACE_BASE_ADDR)
#define K_STACK_SIZE   (4 * 1024 * 1024)

// kernel space physical address
#define __pa(x) ((uint32_t)(x) - KERNEL_SPACE_BASE_ADDR)
// kernel space virtual address
#define __va(x) ((uint32_t)(x) + KERNEL_SPACE_BASE_ADDR)

#endif