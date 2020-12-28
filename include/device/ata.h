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
typedef struct ATADevice ata_device_t;
typedef struct ATAChannel ata_channel_t;

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


struct Disk {
    // name of this disk
    // for example 'sda'
    char disk_name[8];
    // the ata channel to which this disk belongs
    ata_channel_t *my_channel;
    // each channel can have at most two devices
    // 0 for master and 1 for slave
    uint8_t dev_no;
    // at most 4 primary partitions
    partition_t prim_parts[4];
    // there can be infinitely many logic partitions
    // myOS supports at most 8 logic partitions
    partition_t logic_parts[8];
    // whether this device actually exists
    bool_t existed;
};


struct ATADevice {
    // device name
    // for example 'sda', 'sr'
    char dev_name[8];
    // the ata channel to which this device belongs
    ata_channel_t *my_channel;
    // each channel can have at most two devices
    // 0 for master and 1 for slave
    uint8_t dev_no;
    // whether this device actually exists
    bool_t existed;
};


struct ATAChannel {
    // name of this ata channel
    // for example 'ata0' or 'ide0'
    char chan_name[8];
    // starting port number of this channel
    uint16_t port_base;
    // interrupt request number used by this channel
    uint8_t irq_no;
    mutex_t chan_lock;
    // 向硬盘发完命令后等待来自硬盘的中断
    bool_t expecting_intr;
    // 硬盘处理完成.线程用这个信号量来阻塞自己,由硬盘完成后产生的中断将线程唤醒
    sem_t disk_done;
    disk_t devices[2];
};


// init ata devices (hard drives)
void ata_init();

#endif
