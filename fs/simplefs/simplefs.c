/**
 * @file simplefs.c
 */

#include <common/debug.h>

#include <lib/list.h>
#include <lib/kprintf.h>
#include <lib/bitmap.h>

#include <mm/kvmm.h>

#include <device/ata.h>
#include <device/tty.h>

#include <fs/simplefs/simplefs.h>

#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT (8 * SECTOR_SIZE)
#define SIMPLE_FS_MAGIC 0x12345678
#define MAX_FILENAME_LENGTH 23
#define BOOT_BLOCK_SEC_CNT 1
#define SUPER_BLOCK_SEC_CNT 1
#define MAX_FILE_CNT 4096

typedef struct {
    // current file pointer position
    uint32_t file_pos;
    // flags
    uint32_t file_flags;
} file_t;

typedef struct {
    // lba of the first data sector
    uint32_t first_lba;
    // number of bytes in this file
    uint32_t size;
    // filename
    char filename[MAX_FILENAME_LENGTH + 1];
} __attr_packed file_descriptor_t;

typedef struct {
    uint32_t fs_type;
    // sectors used by simplefs
    uint32_t start_lba;
    uint32_t sec_cnt;

    // an on disk array of file descriptors
    uint32_t file_desc_table_start_lba;
    uint32_t file_desc_table_sec_cnt;

    // an on-disk bitmap for used/free sectors
    uint32_t sector_btmp_start_lba;
    uint32_t sector_btmp_sec_cnt;

    // sectors with lba >= data_start_lba are free to use for files
    uint32_t data_start_lba;

} __attr_packed super_block_t;

// implements partition_t.fs_sturct
typedef struct {
    super_block_t *sb;
    btmp_t sector_btmp;
} simplefs_struct_t;

static void print_simplefs(const simplefs_struct_t *simplefs) {
    super_block_t *sb = simplefs->sb;
    kprintf(
        KPL_DEBUG,
        "    fs_type: 0x%x\n"
        "    start_lba: 0x%x\n"
        "    sec_cnt: 0x%x\n"
        "    file_desc_table_start_lba: 0x%x\n"
        "    file_desc_table_sec_cnt: 0x%x\n"
        "    sector_btmp_start_lba: 0x%x\n"
        "    sector_btmp_sec_cnt: 0x%x\n"
        "    data_start_lba: 0x%x\n",
        sb->fs_type, sb->start_lba, sb->sec_cnt,
        sb->file_desc_table_start_lba, sb->file_desc_table_sec_cnt,
        sb->sector_btmp_start_lba, sb->sector_btmp_sec_cnt,
        sb->data_start_lba
    );
    btmp_t *sector_btmp = &(simplefs->sector_btmp);
    print_btmp(sector_btmp);
}

#define __simplefs_field(part, member) (((simplefs_struct_t *)(part->fs_struct))->member)


extern list_t partition_list;

// simple fs only operates the first partition
static partition_t *part;

static int format_partition(uint8_t *buf, super_block_t *sb) {
    ASSERT(part != NULL);
    sb->fs_type = SIMPLE_FS_MAGIC;
    sb->start_lba = part->start_lba;
    sb->sec_cnt = part->sec_cnt;

    sb->file_desc_table_start_lba = sb->start_lba + 2;
    sb->file_desc_table_sec_cnt = ROUND_UP_DIV(MAX_FILE_CNT, SECTOR_SIZE / sizeof(file_descriptor_t));

    sb->sector_btmp_start_lba = sb->file_desc_table_start_lba + sb->file_desc_table_sec_cnt;
    sb->sector_btmp_sec_cnt = ROUND_UP_DIV(sb->sec_cnt, SECTOR_SIZE_IN_BIT);

    sb->data_start_lba = sb->sector_btmp_start_lba + sb->sector_btmp_sec_cnt;

    kprintf(KPL_DEBUG, "%s info: \n", part->part_name);
    kprintf(
        KPL_DEBUG,
        "    fs_type: 0x%x\n"
        "    start_lba: 0x%x\n"
        "    sec_cnt: 0x%x\n"
        "    file_desc_table_start_lba: 0x%x\n"
        "    file_desc_table_sec_cnt: 0x%x\n"
        "    sector_btmp_start_lba: 0x%x\n"
        "    sector_btmp_sec_cnt: 0x%x\n"
        "    data_start_lba: 0x%x\n",
        sb->fs_type, sb->start_lba, sb->sec_cnt,
        sb->file_desc_table_start_lba, sb->file_desc_table_sec_cnt,
        sb->sector_btmp_start_lba, sb->sector_btmp_sec_cnt,
        sb->data_start_lba
    );

    memcpy(sb, buf, sizeof(super_block_t));
    kprintf(KPL_NOTICE, "  Writing super block to disk... ");
    ata_write(part->my_disk, part->start_lba + 1, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, sizeof(super_block_t));

    uint32_t bit_len = sb->sector_btmp_sec_cnt * SECTOR_SIZE_IN_BIT;

    // [0, sb->data_start_lba) -> 1
    // [sb->data_start_lba, sb->sec_cnt) -> 0
    // [sb->sec_cnt, bit_len) -> 1

    ASSERT(sb->data_start_lba <= SECTOR_SIZE_IN_BIT);

    kprintf(KPL_NOTICE, "Used sectors: [0x%X, 0x%X){%x}\n", sb->start_lba, sb->data_start_lba, sb->data_start_lba - sb->start_lba);
    kprintf(KPL_NOTICE, "Free sectors: [0x%X, 0x%X){%x}\n", sb->data_start_lba, sb->start_lba + sb->sec_cnt, sb->start_lba + sb->sec_cnt - sb->data_start_lba);
    kprintf(KPL_NOTICE, "None sectors: [0x%X, 0x%X){%x}\n", sb->start_lba + sb->sec_cnt, sb->start_lba + bit_len, bit_len - sb->sec_cnt);

    kprintf(KPL_NOTICE, "  Writing sector bitmap to disk... ");
    for (uint32_t i = 0; i < sb->data_start_lba - sb->start_lba; i++) {
        buf[i / 8] |= (1 << (i % 8));
    }
    if (sb->sector_btmp_sec_cnt > 1) {
        kprintf(KPL_DEBUG, "ata_write lba = 0x%X\n", sb->sector_btmp_start_lba);
        ata_write(part->my_disk, sb->sector_btmp_start_lba, buf, 1);
        memset(buf, 0, SECTOR_SIZE);
    }

    uint32_t offset = (sb->sector_btmp_sec_cnt - 1) * SECTOR_SIZE_IN_BIT;
    for (uint32_t i = sb->sec_cnt; i < bit_len; i++) {
        buf[(i - offset) / 8] |= (1 << (i % 8));
    }
    kprintf(KPL_DEBUG, "ata_write lba = 0x%X\n", sb->sector_btmp_start_lba + sb->sector_btmp_sec_cnt - 1);
    ata_write(part->my_disk, sb->sector_btmp_start_lba + sb->sector_btmp_sec_cnt - 1, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");

    kprintf(KPL_NOTICE, "Done formatting partition %s with simplefs\n", part->part_name);
}

void simplefs_init() {
    part = __container_of(partition_t, part_tag, list_front(&partition_list));
    if (part == NULL) {
        PANIC("simplefs failed to init: no first partition!");
    }
    uint8_t *buf = kmalloc(SECTOR_SIZE);
    super_block_t *sb = kmalloc(sizeof(super_block_t));
    simplefs_struct_t *simplefs = kmalloc(sizeof(simplefs_struct_t));
    if (buf == NULL || sb == NULL || simplefs == NULL) {
        PANIC("simplefs failed to init: no mem!");
    }
    ata_read(part->my_disk, part->start_lba + 1, buf, 1);
    if (*(uint32_t *)buf != SIMPLE_FS_MAGIC) {
        format_partition(buf, sb);
    } else {
        memcpy(buf, sb, sizeof(super_block_t));
    }

    // mount part
    uint32_t byte_len = sb->sector_btmp_sec_cnt * SECTOR_SIZE;
    simplefs->sector_btmp.bits_ = kmalloc(byte_len);
    if (simplefs->sector_btmp.bits_ == NULL) {
        PANIC("simplefs failed to init: no mem!");
    }
    ata_read(
        part->my_disk, sb->sector_btmp_start_lba,
        simplefs->sector_btmp.bits_, sb->sector_btmp_sec_cnt
    );
    bitmap_reinit(&(simplefs->sector_btmp), byte_len);
    simplefs->sb = sb;
    part->fs_struct = simplefs;
    kfree(buf);
    print_simplefs(simplefs);
}

int sys_open(const char *pathname, uint32_t flags) {
    return -1;
}

int sys_close(int fd) {
    return -1;
}

int sys_read(int fd, void *buffer, size_t count) {
    kprintf(KPL_DEBUG, "sys_read: fd=%d\n", fd);
    if (fd == FD_STDIN) {
        int idx = 0;
        char *buff = (char *)buffer;
        while (idx < count) {
            // sys_read will only get printable chars, \n and \b
            char c = get_printable_char(tty_getkey_curr());
            if (c == '\0') {
                continue;
            }
            buff[idx++] = c;
        }
        return idx;
    }
    return -1;
    // file_t *file = lfd2file(fd);
    // if (
    //     file == NULL ||
    //     file->file_tp != FT_REGULAR ||
    //     file->file_flags & O_WRONLY
    // ) {
    //     return -FSERR_BADFD;
    // }
    // return file_read(curr_part, file, buffer, count);
}

int sys_write(int fd, void *buffer, size_t count) {
    if (fd == FD_STDOUT) {
        return tty_puts(
            get_current_thread()->tty_no, buffer, count, CONS_BLACK, CONS_GRAY
        );
    } else if (fd == FD_STDERR) {
        return tty_puts(
            get_current_thread()->tty_no, buffer, count, CONS_BLACK, CONS_GRAY
        );
    }

    return -1;
    // file_t *file = lfd2file(fd);
    // if (
    //     file == NULL ||
    //     file->file_tp != FT_REGULAR ||
    //     !(file->file_flags & (O_WRONLY | O_RDWR))
    // ) {
    //     return -FSERR_BADFD;
    // }
    // return file_write(curr_part, file, buffer, count);
}