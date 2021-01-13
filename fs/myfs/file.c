#include <fs/myfs/file.h>
#include <fs/myfs/inode.h>
#include <device/ata.h>
#include <thread/thread.h>
#include <fs/myfs/fs_types.h>

static file_t file_table[MAX_FILE_OPEN];

int file_table_get_free_slot() {
    for (int i = 0; i < MAX_FILE_OPEN; i++) {
        if (file_table[i].inode == NULL) {
            return i;
        }
    }
    return -1;
}

int inode_alloc(partition_t *part) {
    ASSERT(part != NULL);
    int idx = bitmap_scan(&(part->inode_btmp), 1);
    if (idx == -1) {
        return -1;
    }
    bitmap_set(&(part->inode_btmp), idx, 1);
    return idx;
}

int block_alloc(partition_t *part) {
    ASSERT(part != NULL);
    int idx = bitmap_scan(&(part->block_btmp), 1);
    if (idx == -1) {
        return -1;
    }
    bitmap_set(&(part->block_btmp), idx, 1);
    return idx;
}

void inode_btmp_sync(partition_t *part, int bit_idx) {
    uint32_t sec_off = bit_idx / SECTOR_SIZE_IN_BIT;
    uint32_t byte_off = sec_off * SECTOR_SIZE;

    uint32_t lba = part->sb->inode_btmp_start_lba + sec_off;
    void *data = part->inode_btmp.bits_ + byte_off;
    dirty_blocks_add(part, lba, data);
}

void block_btmp_sync(partition_t *part, int bit_idx) {
    uint32_t sec_off = bit_idx / SECTOR_SIZE_IN_BIT;
    uint32_t byte_off = sec_off * SECTOR_SIZE;

    uint32_t lba = part->sb->block_btmp_start_lba + sec_off;
    void *data = part->block_btmp.bits_ + byte_off;
    dirty_blocks_add(part, lba, data);
}
