#ifndef __MEM_H__
#define __MEM_H__

#include <multiboot/multiboot.h>
#include <mm/pager.h>

// myOS requires at least 254 MiB free mem
#define MEM_LIMIT 254 * 1024 * 1024

// myOS never use low 1 MiB memory
#define AVAIL_MEM_BASE 0x00100000

#define AVAIL_PAGE_BASE (AVAIL_MEM_BASE / PAGE_SIZE)


void print_mem_info(multiboot_info_t *mbi);

// memory management init
void mm_init(multiboot_info_t *mbi);

#endif