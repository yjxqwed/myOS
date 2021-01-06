#include <fs/myfs/fs.h>
#include <fs/myfs/superblock.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/dir.h>
#include <device/ata.h>
#include <common/types.h>
#include <common/debug.h>
#include <mm/kvmm.h>
#include <myos.h>
#include <kprintf.h>
#include <list.h>
#include <string.h>

static int format_partition(partition_t *part) {
    ASSERT(part != NULL);
    kprintf(KPL_NOTICE, "Started to format partition %s\n", part->part_name);
    // boot block occupies 1 sector
    uint32_t boot_block_secs = 1;
    // super block occupies 1 sector
    uint32_t super_block_secs = 1;
    super_block_t *sb = (super_block_t *)kmalloc(sizeof(super_block_t));
    if (sb == NULL) {
        return -ERR_MEMORY_SHORTAGE;
    }
    sb->fs_type = 0x19971125;
    sb->sec_cnt = part->sec_cnt;
    sb->inode_cnt = MAX_FILE_CNT_PER_PART;
    sb->part_start_lba = part->start_lba;

    uint32_t curr_free_secs_start_lba =
        part->start_lba + boot_block_secs + super_block_secs;

    sb->inode_btmp_start_lba = curr_free_secs_start_lba;
    sb->inode_btmp_sec_cnt = ROUND_UP_DIV(sb->inode_cnt, SECTOR_SIZE_IN_BIT);
    curr_free_secs_start_lba += sb->inode_btmp_sec_cnt;

    sb->inode_table_start_lba = curr_free_secs_start_lba;
    sb->inode_table_sec_cnt =
        ROUND_UP_DIV(sb->inode_cnt * sizeof(inode_t), SECTOR_SIZE);
    curr_free_secs_start_lba += sb->inode_table_sec_cnt;

    sb->block_btmp_start_lba = curr_free_secs_start_lba;
    sb->block_btmp_sec_cnt = ROUND_UP_DIV(sb->sec_cnt, SECTOR_SIZE_IN_BIT);
    curr_free_secs_start_lba += sb->block_btmp_sec_cnt;

    sb->data_start_lba = curr_free_secs_start_lba;
    sb->root_inode_no = 0;
    sb->dir_entry_size = sizeof(dir_entry_t);

    kprintf(KPL_NOTICE, "%s info: \n", part->part_name);
    kprintf(
        KPL_NOTICE,
        "    fs_type: 0x%x\n"
        "    part_start_lba: 0x%x\n"
        "    sec_cnt: 0x%x\n"
        "    inode_cnt: 0x%x\n"
        "    inode_btmp_start_lba: 0x%x\n"
        "    inode_btmp_sec_cnt: 0x%x\n"
        "    inode_table_start_lba: 0x%x\n"
        "    inode_table_sec_cnt: 0x%x\n"
        "    block_btmp_start_lba: 0x%x\n"
        "    block_btmp_sec_cnt: 0x%x\n"
        "    data_start_lba: 0x%x\n",
        sb->fs_type, sb->part_start_lba, sb->sec_cnt, sb->inode_cnt,
        sb->inode_btmp_start_lba, sb->inode_btmp_sec_cnt,
        sb->inode_table_start_lba, sb->inode_table_sec_cnt,
        sb->block_btmp_start_lba, sb->block_btmp_sec_cnt,
        sb->data_start_lba
    );

    // write super block to disk
    kprintf(KPL_NOTICE, "  Writing super block to disk... ");
    ata_write(part->my_disk, part->start_lba + boot_block_secs, sb, 1);
    kprintf(KPL_NOTICE, "  Done!\n");

    uint32_t buf_size = MAX(sb->inode_btmp_sec_cnt, sb->inode_table_sec_cnt);
    buf_size = MAX(buf_size, sb->block_btmp_sec_cnt);
    buf_size *= SECTOR_SIZE;
    uint8_t *buf = (uint8_t *)kmalloc(buf_size);
    if (buf == NULL) {
        kfree(sb);
        return -ERR_MEMORY_SHORTAGE;
    }

    // init inode_btmp
    // inode 0 has been assigned to root
    buf[0] = 0x01;
    kprintf(KPL_NOTICE, "  Writing inode bitmap to disk... ");
    ata_write(part->my_disk, sb->inode_btmp_start_lba, buf, sb->inode_btmp_sec_cnt);
    kprintf(KPL_NOTICE, "  Done!\n");

    buf[0] = 0;
    // init inode_table
    inode_t *inode = (inode_t *)buf;
    // inode is for root
    inode->i_no = 0;
    // root is a dir and it contains . (self) and .. (parent dir) by default
    // as its dir entries
    inode->i_size = sb->dir_entry_size * 2;
    inode->i_blocks[0] = sb->data_start_lba;
    kprintf(KPL_NOTICE, "  Writing inode table to disk... ");
    ata_write(part->my_disk, sb->inode_table_start_lba, buf, sb->inode_table_sec_cnt);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, sizeof(inode_t));

    /** init block_btmp */
    uint32_t byte_len = sb->block_btmp_sec_cnt * SECTOR_SIZE;
    uint32_t bit_len = byte_len * 8;
    // num of sectors used by meta data
    uint32_t used_sectors = sb->data_start_lba - sb->part_start_lba;
    // non-existed sectors
    uint32_t none_sectors = bit_len - sb->sec_cnt;

    // the first used_sectors bits and last none_sectors bits should be 1
    // others should be 0
    uint32_t i;
    for (i = 0; i < used_sectors / 8; i++) {
        buf[i] = 0xff;
    }
    for (int j = 0; j < used_sectors % 8; j++) {
        buf[i] |= (0x1 << j);
    }

    for (i = 0; i < none_sectors / 8; i++) {
        buf[byte_len - 1 - i] = 0xff;
    }
    for (int j = 0; j < none_sectors % 8; j++) {
        buf[byte_len - 1 - i] |= (0x01 << (7 - j));
    }

    // the first block is used by root
    int byte_idx = (sb->data_start_lba - sb->part_start_lba) / 8;
    int bit_idx = (sb->data_start_lba - sb->part_start_lba) % 8;
    buf[byte_idx] |= (0x01 << bit_idx);

    kprintf(KPL_NOTICE, "  Writing block bitmap to disk... ");
    ata_write(part->my_disk, sb->block_btmp_start_lba, buf, sb->block_btmp_sec_cnt);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, buf_size);

    // init root data
    dir_entry_t *de = (dir_entry_t *)buf;
    // init .
    strcpy(".", de[0].filename);
    de[0].i_no = 0;
    de[0].f_type = FT_DIRECTORY;

    // init ..
    strcpy("..", de[1].filename);
    de[1].i_no = 0;
    de[1].f_type = FT_DIRECTORY;

    kprintf(KPL_NOTICE, "  Writing root to disk... ");
    ata_write(part->my_disk, sb->data_start_lba, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");

    // kprintf(KPL_DEBUG, "used: %x; none: %x; byte_len: 0x%x\n", used_sectors, none_sectors, byte_len);
    kfree(sb);
    kfree(buf);
    kprintf(KPL_NOTICE, "Done formatting partition %s\n", part->part_name);
    return ERR_NO_ERR;
}


extern ata_channel_t channels[2];
extern list_t partition_list;

void fs_init() {
    list_node_t *p;
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        format_partition(part);
    }
}
