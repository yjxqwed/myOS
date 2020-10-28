#ifndef __MEM_H__
#define __MEM_H__

#include <multiboot/multiboot.h>
#include <mm/pager.h>
#include <bitmap.h>
#include <common/types.h>

// myOS requires at least 254 MiB free mem
// 254 = 256 - low 1 MiB - high 1 MiB
#define MEM_LIMIT 254 * 1024 * 1024

// myOS never uses low 1 MiB memory
#define AVAIL_MEM_BASE 0x00100000
#define AVAIL_PAGE_BASE (AVAIL_MEM_BASE / PAGE_SIZE)

// myOS reserves high 1 MiB ~ 15 MiB mem
#define K_RESERVED_MEM 0x00F00000

// mem below AVAIL_MEM_BASE is reserved for BIOS and GRUB
// myOS reserves K_RESERVED_MEM amount of mem for system data structures use
// mem above FREE_MEM_BASE is free to use for allocating
#define FREE_MEM_BASE (AVAIL_MEM_BASE + K_RESERVED_MEM)

// myOS only uses up to 512 MiB physical mem
#define K_NEEDED_MEM 0x20000000


void print_mem_info(multiboot_info_t *mbi);

// memory management init
void mm_init(multiboot_info_t *mbi);



#endif