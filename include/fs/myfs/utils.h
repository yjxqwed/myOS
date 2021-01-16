#ifndef __MYFS_UTILS_H__
#define __MYFS_UTILS_H__

#include <fs/myfs/fs_types.h>
#include <common/types.h>
#include <device/ata.h>

/**
 * @brief path descriptor
 */
typedef struct {
    // whether path is an absolute path (from root)
    bool_t abs;
    // depth of path
    int depth;
    // valid depth (to determine invalid filenames)
    int valid_depth;
    // filenames on path
    struct {
        char filename[MAX_FILE_NAME_LENGTH + 1];
        file_type_e ft;
    } path[MAX_PATH_DEPTH];
} path_info_t;

/**
 * @brief analyze path
 */
int analyze_path(const char *pathname, path_info_t *pi);


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
void inode_btmp_sync(partition_t *part, int inode_bit_idx);

/**
 * @brief add dirty blocks to sync
 */
void block_btmp_sync(partition_t *part, int blk_bit_idx);


#endif
