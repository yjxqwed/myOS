#ifndef __MYFS_INODE_H__
#define __MYFS_INODE_H__

#include <common/types.h>
#include <list.h>

/**
 * inode is for information node or index node
 * it is the metadata of any file(directory) on disk
 */

typedef struct INode {
    // inode number
    uint32_t i_no;

    /**
     * If this inode is for a file, i_size is the size of the file
     * If this inode is for a dir, i_size if the sum of all dir entries' size
     */
    uint32_t i_size;
    // number of times of this file being opened
    uint32_t i_open_times;

    bool_t write_deny;

    // num of sectors of a data block
    uint16_t block_size;

    // i_blocks is the array of data blocks, each data block contain some sectors
    // i_blocks[0] ~ i_blocks[11] are direct sectors
    // i_blocks[12] is the one-level indirect sector
    // i_blocks[13] is the two-level indirect sector
    // i_blocks[14] is the three-level indirect sector
    uint32_t i_blocks[15];

    // for linked list to use
    list_node_t i_tag;
} inode_t;

#endif
