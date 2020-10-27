#ifndef __MEM_H__
#define __MEM_H__

#include <multiboot/multiboot.h>

void print_mem_info(multiboot_info_t *mbi);

// myOS requires at least 254 MiB free mem
#define MEM_LIMIT 254 * 1024 * 1024
// memory management init
void mm_init(multiboot_info_t *mbi);

#endif