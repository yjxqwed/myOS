#ifndef __MYFS_INODE_H__
#define __MYFS_INODE_H__

#include <common/types.h>
#include <device/ata.h>  // for partition_t
#include <fs/myfs/fs_types.h>  // for im_inode_t


/**
 * inode is for information node or index node
 * it is the metadata of any file(directory) on disk
 * sizeof(inode_t) should be a divisor of 512
 */

#define NR_BLOCKS_PER_INODE 14

typedef struct INode {
    // inode number
    uint32_t i_no;

    /**
     * If this inode is for a file, i_size is the size of the file
     * If this inode is for a dir, i_size is the sum of all dir entries' size
     */
    uint32_t i_size;

    /**
     * i_blocks is the array of lbas of data blocks
     * i_blocks[0:12] are direct blocks
     * i_blocks[12:14] are the second level blocks
     */
    uint32_t i_blocks[NR_BLOCKS_PER_INODE];
} __attr_packed inode_t;


// the in memory wrapper of inode
typedef struct InMemoryINode {
    // on disk inode
    inode_t inode;
    // number of times of this file being opened
    uint32_t i_open_times;
    // one file can be written by a single writer.
    bool_t write_deny;
    // for linked list to use
    list_node_t i_tag;
    // if true, need to sync
    bool_t dirty;
    /// io buffer
    void *io_buffer;
} im_inode_t;

/**
 * @brief sync inode(table) to disk
 * @param part partition
 * @param im_inode in memory representative of the inode
 * @param buffer IO buffer created by the caller (at least BLOCK_SIZE B)
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
