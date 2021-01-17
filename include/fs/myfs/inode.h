#ifndef __MYFS_INODE_H__
#define __MYFS_INODE_H__

#include <common/types.h>
#include <common/utils.h>
#include <list.h>
#include <device/ata.h>
#include <fs/myfs/fs_types.h>

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
