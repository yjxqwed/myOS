#ifndef __PAGER_H__
#define __PAGER_H__

#include <common/types.h>
#include <multiboot/multiboot.h>

// init paging system
void init_paging();

// clear low memory mapping after enabling paging
void clear_low_mem_mapping();

void install_boot_pg(void);
#endif