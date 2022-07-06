#ifndef __ATA_H__
#define __ATA_H__

/**
 * @file include/device/ata.h
 * @brief ATA (Advanced Technology Atachment)
 *        IDE (Integreted Device Electronics)
 *        Basically, they are terms describing Hard Drives.
 */

#include "types.h"
#include <stdio.h>
// #include <thread/sync.h>
// #include <lib/bitmap.h>
// #include <lib/list.h>

typedef struct Partition partition_t;
typedef struct Disk disk_t;

// an in memory structure for a partition
struct Partition {
    uint32_t start_lba;
    uint32_t sec_cnt;
    disk_t *my_disk;
    char part_name[33];
    // used by the filesystem in this partition
    void *fs_struct;
};

/**
 * @brief get partition by name
 * 
 * @return partition_t* NULL if no such partition
 */
partition_t *get_partition(const char *part_name);


struct Disk {
    FILE *fp;
    uint32_t sectors;
    partition_t prim_parts[4];
    uint8_t p_no;
    partition_t logic_parts[8];
    uint8_t l_no;
    char disk_name[32];
};

disk_t *disk_open(const char *filename);
int disk_close(disk_t *disk);
void print_disk(const disk_t *disk);

void ata_read(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt);
void ata_write(disk_t *hd, uint32_t lba, const void *buf, uint32_t sec_cnt);

#endif
