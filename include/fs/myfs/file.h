#ifndef __MYFS_FILE_H__
#define __MYFS_FILE_H__

#include <fs/myfs/inode.h>
#include <common/types.h>

// std io fds
enum {
    FD_STDIN = 0,
    FD_STDOUT,
    FD_STDERR
};

typedef struct File {
    // inode of this file
    im_inode_t *im_inode;
    // current file pointer position
    uint32_t file_pos;
    // flags
    uint32_t file_flags;
} file_t;

// myfs supports at most 32 open files at any moment
#define MAX_FILE_OPEN 32


/**
 * @brief create a file under pdir for calling process
 * @return process local fd; errno (less than 0) if failed
 */
int file_create(
    partition_t *part, dir_t *pdir, const char *filename,
    uint32_t flags, void *io_buf
);

/**
 * @brief open a file by its inode number
 * @return fd if success; negative errno if failure
 */
int file_open(partition_t *part, int i_no, uint32_t flags);

#endif
