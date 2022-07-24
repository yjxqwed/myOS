#ifndef __MM_INFO_H__
#define __MM_INFO_H__

#include <common/types.h>

typedef struct {
    uint32_t total_mem_installed;
    uint32_t used_mem;
    uint32_t free_mem;
} mm_info_t;


#endif
