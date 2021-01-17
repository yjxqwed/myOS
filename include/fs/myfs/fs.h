#ifndef __MYFS_FILESYSTEM_H__
#define __MYFS_FILESYSTEM_H__

#include <common/types.h>

void fs_init();

/**
 * @brief open a file
 */
int sys_open(const char *pathname, uint32_t flags);

#endif
