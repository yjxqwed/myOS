#ifndef __MYFS_UTILS_H__
#define __MYFS_UTILS_H__

#include <fs/myfs/fs_types.h>
#include <common/types.h>
#include <device/ata.h>

#define __myfs_field(part, member) (((myfs_struct_t *)(part->fs_struct))->member)

/**
 * @brief path descriptor
 */
typedef struct {
    // whether path is an absolute path (from root)
    bool_t abs;
    // isdir => path is a dir; !isdir => not sure
    bool_t isdir;
    // depth of path
    int depth;
    // filenames on path
    char path[MAX_PATH_DEPTH][MAX_FILE_NAME_LENGTH + 1];
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
 * @brief recliam i_no
 */
void inode_reclaim(partition_t *part, int i_no);

/**
 * @brief assign a block from a partition
 */
int block_alloc(partition_t *part);

/**
 * @brief recliam blk_no (not lba)
 */
void block_reclaim(partition_t *part, int blk_no);

/**
 * @brief add dirty blocks to sync
 */
void inode_btmp_sync(partition_t *part, int inode_bit_idx);

/**
 * @brief add dirty blocks to sync
 */
void block_btmp_sync(partition_t *part, int blk_bit_idx);

/**
 * @brief read block(s)
 */
void partition_block_read(partition_t *part, uint32_t blk_id, void *buf, uint32_t blk_cnt);

/**
 * @brief write block(s)
 */
void partition_block_write(partition_t *part, uint32_t blk_id, void *buf, uint32_t blk_cnt);

#endif
