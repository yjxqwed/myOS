#ifndef __PAGER_H__
#define __PAGER_H__

#include <common/types.h>
#include <multiboot/multiboot.h>

void install_boot_pg(void);
void kernel_init_paging(void);
#endif