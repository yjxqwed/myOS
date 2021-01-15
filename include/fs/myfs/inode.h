#ifndef __MYFS_INODE_H__
#define __MYFS_INODE_H__

#include <common/types.h>
#include <common/utils.h>
#include <list.h>
#include <device/ata.h>

/**
 * inode is for information node or index node
 * it is the metadata of any file(directory) on disk
 * sizeof(inode_t) should be a divisor of 512
 */

#define NR_BLOCKS_PER_INODE 15

typedef struct INode {
    // inode number
    uint32_t i_no;

    /**
     * If this inode is for a file, i_size is the size of the file
     * If this inode is for a dir, i_size if the sum of all dir entries' size
     */
    uint32_t i_size;

    /**
     * i_blocks is the array of data blocks, each data block contain some sectors
     * i_blocks[0] ~ i_blocks[11] are direct sectors
     * i_blocks[12] is the one-level indirect sector
     * i_blocks[13] is the two-level indirect sector
     * i_blocks[14] is the three-level indirect sector
     */
    uint32_t i_blocks[NR_BLOCKS_PER_INODE];
} inode_t;


// the in memory wrapper of inode
typedef struct InMemoryInode {
    // on disk inode
    inode_t inode;
    // number of times of this file being opened
    uint32_t i_open_times;
    bool_t write_deny;
    // for linked list to use
    list_node_t i_tag;
    // if true, need to sync
    bool_t dirty;
} im_inode_t;

/**
 * @brief sync inode(table) to disk
 * @param part partition
 * @param im_inode in memory representative of the inode
 * @param buffer IO buffer, 512B at least
 */
void inode_sync(partition_t *part, im_inode_t *im_inode, void *buffer);

/**
 * @brief open an inode
 * @param part partition
 * @param i_no inode number
 * @return in memory representative of the inode
 */
im_inode_t *inode_open(partition_t *part, uint32_t i_no);

/**
 * @brief close an open inode
 * @param im_inode in memory representative of the inode
 */
void inode_close(im_inode_t *im_inode);

/**
 * @brief init an im_inode
 */
void im_inode_init(im_inode_t *im_inode, int i_no);

#endif
