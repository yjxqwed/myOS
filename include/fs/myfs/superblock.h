#ifndef __MYFS_SUPERBLOCK_H__
#define __MYFS_SUPERBLOCK_H__

typedef struct SuperBlock super_block_t;

#include <common/types.h>
#include <common/utils.h>

/**
 * Super block is a per partition structure.
 * It describes the file system of this partition.
 * The location of every super block is fixed.
 */
struct SuperBlock {
    // a magic number indicating the file system type of this partition
    uint32_t fs_type;
    // number of sectors of this partition
    uint32_t sec_cnt;
    // number of inodes of this partition
    uint32_t inode_cnt;
    // start lba of this partition
    uint32_t part_start_lba;

    // block bitmap start lba
    uint32_t block_btmp_start_lba;
    // number of sectors used by block bitmap
    uint32_t block_btmp_sec_cnt;

    // inode bitmap start lba
    uint32_t inode_btmp_start_lba;
    // number of sectors used by inode bitmap
    uint32_t inode_btmp_sec_cnt;

    // inode table start lba
    uint32_t inode_table_start_lba;
    // number of sectors used by inode table
    uint32_t inode_table_sec_cnt;

    // data region start lba
    uint32_t data_start_lba;
    // inode number of root directory of this partition
    uint32_t root_inode_no;
    // size of a directory entry
    uint32_t dir_entry_size;

    // reversed
    uint8_t unused[460];
} __attr_packed;

#endif
