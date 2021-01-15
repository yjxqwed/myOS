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
 * @brief assign an inode from a partition
 */
int inode_alloc(partition_t *part);

/**
 * @brief assign a block from a partition
 */
int block_alloc(partition_t *part);

/**
 * @brief add dirty blocks to sync
 */
void inode_btmp_sync(partition_t *part, int bit_idx);

/**
 * @brief add dirty blocks to sync
 */
void block_btmp_sync(partition_t *part, int bit_idx);


#endif
