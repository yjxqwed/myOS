#ifndef __MYFS_FILESYSTEM_H__
#define __MYFS_FILESYSTEM_H__

#include <common/types.h>
#include <fs/myfs/fs_types.h>

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


/**
 * @brief read from fd
 * @return number of bytes read
 */
int sys_read(int fd, void *buffer, size_t count);


/**
 * @brief set file position pointer
 * @return file position pointer or -errno
 */
off_t sys_lseek(int fd, off_t offset, int whence);


/**
 * @brief rewind dir by its fd
 */
int sys_rewinddir(int fd);


/**
 * @brief make a dir according to pathname
 * @return 0 or -errno
 */
int sys_mkdir(const char *pathname);


/**
 * @brief get current working directory;
 *        if size < needed length, will fail
 * @return 0 or -errno
 */
int sys_getcwd(char *buf, size_t size);


/**
 * @brief change working directory
 * @return 0 or -errno
 */
int sys_chdir(const char *path);


/*** debug utilities ***/
void print_fd_table();
void print_open_inodes();

#endif
