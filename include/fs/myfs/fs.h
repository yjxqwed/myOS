#ifndef __MYFS_FILESYSTEM_H__
#define __MYFS_FILESYSTEM_H__

#include <common/types.h>

void fs_init();

/**
 * @brief open a file
 */
int sys_open(const char *pathname, uint32_t flags);

/**
 * @brief close a file
 */
int sys_close(int fd);


/**
 * @brief get dir entries
 * @return number of bytes read
 */
int sys_getdents(int fd, void *buffer, size_t count);


/**
 * @brief write to fd
 * @return number of bytes written
 */
int sys_write(int fd, void *buffer, size_t count);

void print_fd_table();
void print_open_inodes();

#endif
