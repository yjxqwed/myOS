#ifndef __ATA_H__
#define __ATA_H__

/**
 * @file include/device/ata.h
 * @brief ATA (Advanced Technology Atachment)
 *        IDE (Integreted Device Electronics)
 *        Basically, they are terms describing Hard Drives.
 */

#include <common/types.h>
#include <thread/sync.h>
#include <lib/bitmap.h>
#include <lib/list.h>

typedef struct Partition partition_t;
typedef struct Disk disk_t;
typedef struct ATADevice ata_device_t;
typedef struct ATAChannel ata_channel_t;
typedef __pair(uint32_t, void *) lba_data_pair_t;

#define NR_DIRTY_BLOCKS 16

// an in memory structure for a partition
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

    /**
     * Blocks need to be sync after an operation
     *   first is lba, second is kva to write to disk
     *     write BLOCK_SIZE bytes starting from kva to blocks start from lba
     *   if an entry is not 0, it should be sync
     *   will be checked after an operation
     */
    lba_data_pair_t dirty_blocks[NR_DIRTY_BLOCKS];

    // used by the filesystem in this partition
    void *fs_struct;
};

/**
 * @brief get partition by name
 * 
 * @return partition_t* NULL if no such partition
 */
partition_t *get_partition(const char *part_name);

/**
 * @brief add a dirty block to sync
 * @param lba block address on disk
 * @param data data address in memory
 */
void dirty_blocks_add(partition_t *part, uint32_t lba, void *data);

/**
 * @brief sync dirty blocks to disk
 */
void dirty_blocks_sync(partition_t *part);


struct Disk {
    // whether this device actually exists
    bool_t existed;
    // name of this disk
    // for example 'sda'
    char disk_name[8];
    // the ata channel to which this disk belongs
    ata_channel_t *my_channel;
    // each channel can have at most 2 devices
    // 0 for master and 1 for slave
    uint8_t dev_no;
    // number of sectors on this disk
    uint32_t sectors;
    // at most 4 primary partitions
    partition_t prim_parts[4];
    // number of primary partitions
    uint8_t p_no;
    // there can be infinitely many logic partitions
    // myOS supports at most 8 logic partitions
    partition_t logic_parts[8];
    // number of logical partitions
    uint8_t l_no;

    // other info
    char SN[20];
    char MODULE[40];
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
    // name of this ata channel, for example 'ata0' or 'ide0'
    char chan_name[8];
    // starting port number of this channel
    uint16_t port_base;
    // interrupt request number used by this channel
    uint8_t irq_no;
    // for mutually exclusive
    mutex_t chan_lock;
    // whether this channel just sent a command to the device
    // and is waiting for an interrupt
    bool_t expecting_intr;
    // when done sending out a command, use this semaphore to block itself
    // and avoid busy waiting
    sem_t disk_done;
    // every channel has at most 2 devices
    disk_t devices[2];
};


// init ata devices (hard drives)
void ata_init();

/**
 * @brief read from ata device
 * @param hd target disk
 * @param lba start lba to read
 * @param buf buffer to store data
 * @param sec_cnt sector count to read
 */
void ata_read(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt);

/**
 * @brief write to ata device
 * @param hd target disk
 * @param lba start lba to write
 * @param buf buffer of data to write
 * @param sec_cnt sector count to write
 */
void ata_write(disk_t *hd, uint32_t lba, const void *buf, uint32_t sec_cnt);

#endif
