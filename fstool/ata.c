#include "ata.h"
#include "utils.h"
#include <fcntl.h>


// for more info: https://wiki.osdev.org/Partition_Table
struct PartitionTableEntry {
    // 0x80 = bootable; 0x00 = no
    uint8_t bootable;

    // start head/sector/cylinder number of this partition
    // 8bits/6bits/10bits each, 3bytes in total
    uint8_t start_head;
    uint8_t start_sec : 6;
    uint16_t start_cyl : 10;

    // system id
    // 0x05 or 0x0f = extended partition entry
    uint8_t fs_type;

    // end head/sector/cylinder number of this partition
    // 8bits/6bits/10bits each, 3bytes in total
    uint8_t end_head;
    uint8_t end_sec : 6;
    uint16_t end_cyl : 10;

    // start lba of this partitioin
    uint32_t start_lba;
    // number of sectors in this partition
    uint32_t sec_cnt;
} __attr_packed;

typedef struct PartitionTableEntry part_tab_entry_t;

struct BootSector {
    // the first 446 bytes contain the boot code
    uint8_t boot_code[446];
    // the following 64 bytes contain 4 disk partition table entry
    // 16 bytes each
    part_tab_entry_t partition_table[4];
    // the last 2 bytes are 0x55, 0xaa
    uint16_t signature;
} __attr_packed;

typedef struct BootSector boot_sector_t;

void ata_read(disk_t *hd, uint32_t lba, void *buf, uint32_t sec_cnt) {
    fseek(hd->fp, lba * 512, SEEK_SET);
    fread(buf, 512, sec_cnt, hd->fp);
}

void ata_write(disk_t *hd, uint32_t lba, const void *buf, uint32_t sec_cnt) {
    fseek(hd->fp, lba * 512, SEEK_SET);
    fwrite(buf, 512, sec_cnt, hd->fp);
}

partition_t *first_part = NULL;

static void partition_scan(disk_t *hd, uint32_t base_lba) {
    boot_sector_t *bs = kmalloc(sizeof(boot_sector_t));
    static uint32_t primary_ext_base_lba = 0;
    ata_read(hd, base_lba, bs, 1);
    // traverse the 4 entries
    for (int i = 0; i < 4; i++) {
        part_tab_entry_t *pe = &(bs->partition_table[i]);
        if (pe->fs_type == 0) {
            // unused entry
            continue;
        } else if (pe->fs_type == 0x5 || pe->fs_type == 0xf) {
            // extended partition entry
            if (primary_ext_base_lba == 0) {
                primary_ext_base_lba = pe->start_lba;
                partition_scan(hd, pe->start_lba);
            } else {
                partition_scan(hd, primary_ext_base_lba + pe->start_lba);
            }
        } else {
            // normal partition entry
            partition_t *part;
            if (base_lba == 0) {
                // primary partition
                ASSERT(hd->p_no < 4);
                part = &(hd->prim_parts[hd->p_no]);
                ksprintf(part->part_name, "%s%d", hd->disk_name, 1 + hd->p_no++);
            } else {
                // logical partition
                if (hd->l_no >= 8) {
                    kfree(bs);
                    return;
                }
                part = &(hd->logic_parts[hd->l_no]);
                ksprintf(part->part_name, "%s%d", hd->disk_name, 5 + hd->l_no++);
            }
            part->start_lba = base_lba + pe->start_lba;
            part->sec_cnt = pe->sec_cnt;
            part->my_disk = hd;
            if (!first_part) {
                first_part = part;
            }
        }
    }
    kfree(bs);
}

void print_disk(const disk_t *disk) {
    kprintf(KPL_DEBUG, "disk: %s\n", disk->disk_name);
    printf("  primary partition(s): %d\n", disk->p_no);
    for (uint8_t i = 0; i < disk->p_no; i++) {
        const partition_t *part = &(disk->prim_parts[i]);
        printf("    [%s] start = 0x%x, cnt = 0x%x\n", part->part_name, part->start_lba, part->sec_cnt);
    }
    printf("  logical partition(s): %d\n", disk->l_no);
    for (uint8_t i = 0; i < disk->l_no; i++) {
        const partition_t *part = &(disk->logic_parts[i]);
        printf("    [%s] start = 0x%x, cnt = 0x%x\n", part->part_name, part->start_lba, part->sec_cnt);
    }
}

disk_t *disk_open(const char *filename) {
    disk_t *disk = kmalloc(sizeof(disk_t));
    strcpy(filename, disk->disk_name);

    disk->fp = fopen(filename, "rb+");
    // disk->fp = fdopen(fd, "rb+");
    partition_scan(disk, 0);
    return disk;
}

int disk_close(disk_t *disk) {
    fclose(disk->fp);
    kfree(disk);
    return 0;
}