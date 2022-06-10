#ifndef __SIMPLEFS_FILESYSTEM_H__
#define __SIMPLEFS_FILESYSTEM_H__

#include <common/types.h>
#include <common/utils.h>

// open file flags
enum {
    // read only
    O_RDONLY = 0,
    // write only
    O_WRONLY = 1,
    // read & write
    O_RDWR   = 2,
    // create
    O_CREAT  = 4,
};

// for lseek
enum {
    SEEK_SET = 0,
    SEEK_CUR = 1,
    SEEK_END = 2
};

// Reserved FDs
enum {
    FD_STDIN = 0,
    FD_STDOUT,
    FD_STDERR
};

void simplefs_init();

/**
 * @brief open a file
 */
int sys_open(const char *pathname, uint32_t flags);

/**
 * @brief close a file
 */
int sys_close(int fd);

/**
 * @brief read from fd
 * @return number of bytes read
 */
int sys_read(int fd, void *buffer, size_t count);

/**
 * @brief write to fd
 * @return number of bytes written
 */
int sys_write(int fd, void *buffer, size_t count);

typedef int off_t;

/**
 * @brief set file position pointer
 * @return file position pointer or -errno
 */
off_t sys_lseek(int fd, off_t offset, int whence);

#endif