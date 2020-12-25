#ifndef __ATA_H__
#define __ATA_H__

/**
 * @file include/device/ata.h
 * @brief ATA (Advanced Technology Atachment), i.e.
 *        IDE (Integreted Device Electronics).
 *        Basically, they are terms describing Hard Drives.
 */

typedef struct Partition partition_t;
typedef struct Disk disk_t;
typedef struct ATAChannle ata_channel_t;

#include <common/types.h>
#include <thread/sync.h>
#include <lib/bitmap.h>
#include <lib/list.h>

struct Partition {
    // lba is for logical block addressing
    // the start sector number
    uint32_t start_lba;
    // number of sectors of this partition
    uint32_t sec_cnt;
    // the disk to which this partition belongs
    disk_t *my_disk;
    list_node_t part_tag;
    // name of this partition
    char part_name[8];

    struct super_block *sb;
    btmp_t block_btmp;
    btmp_t inode_btmp;
    // list of open inodes
    list_t open_inodes;
};


#endif