#ifndef __PMEM_H__
#define __PMEM_H__

// physical memory management

#include <multiboot/multiboot.h>

// high mem starts at 1MiB
#define HIGH_MEM_BASE 0x00100000


// setup physical memory management system
void setup_memory(multiboot_info_t *mbi);

#endif