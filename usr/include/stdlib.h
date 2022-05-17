#ifndef __USR_STDLIB_H__
#define __USR_STDLIB_H__

#include <common/types.h>

void *malloc(size_t size);
void free(void *addr);
void print_free_blks();

#endif
