#include <fs/myfs/file.h>
#include <fs/myfs/inode.h>
#include <device/ata.h>
#include <thread/thread.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/dir.h>
#include <mm/kvmm.h>
#include <string.h>

static file_t file_table[MAX_FILE_OPEN];

int file_table_get_free_slot() {
    for (int i = 0; i < MAX_FILE_OPEN; i++) {
        if (file_table[i].im_inode == NULL) {
            return i;
        }
    }
    return -1;
}

void file_table_reclaim(int gfd) {
    file_table[gfd].im_inode = NULL;
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

void inode_reclaim(partition_t *part, int i_no) {
    bitmap_set(&(part->inode_btmp), i_no, 0);
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

void block_reclaim(partition_t *part, uint32_t blk_no) {
    bitmap_set(&(part->block_btmp), blk_no, 0);
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

int file_create(partition_t *part, dir_t *pdir, const char *filename, uint32_t flags) {

    ASSERT(strlen(filename) <= MAX_FILE_NAME_LENGTH);

    int rollback = -1, err = FSERR_NOERR;
    // alloc io buffer
    void *io_buf = kmalloc(BLOCK_SIZE);
    // alloc in memory inode struct
    im_inode_t *file_im_ino = kmalloc(sizeof(im_inode_t));

    if (io_buf == NULL || file_im_ino == NULL) {
        err = -FSERR_NOMEM;
        rollback = 0;
        goto __fail__;
    }

    // alloc an inode
    int file_ino = inode_alloc(part);
    if (file_ino == -1) {
        err = -FSERR_NOINODE;
        rollback = 0;
        goto __fail__;
    }
    im_inode_init(file_im_ino, file_ino);

    // alloc a global fd
    int file_gfd = file_table_get_free_slot();
    if (file_gfd = -1) {
        err = -FSERR_NOGLOFD;
        rollback = 1;
        goto __fail__;
    }
    file_table[file_gfd].file_flags = flags;
    file_table[file_gfd].file_pos = 0;
    file_table[file_gfd].im_inode = file_im_ino;

    // write dir entry
    dir_entry_t dir_ent;
    create_dir_entry(filename, file_ino, FT_REGULAR, &dir_ent);
    err = write_dir_entry(part, pdir, &dir_ent, io_buf);
    if (err != FSERR_NOERR) {
        rollback = 2;
        goto __fail__;
    }

    // sync inode bitmap
    inode_btmp_sync(part, file_ino);
    // sync parent inode
    inode_sync(part, pdir->im_inode, io_buf);
    // sync file inode
    inode_sync(part, file_im_ino, io_buf);
    // add file inode to cache
    __list_push_front(&(part->open_inodes), file_im_ino, i_tag);

    kfree(io_buf);
    return FSERR_NOERR;

__fail__:
    ASSERT(err != FSERR_NOERR);
    ASSERT(rollback == 2 || rollback == 1 || rollback == 0);
    switch (rollback) {
        case 2:
            file_table_reclaim(file_gfd);
        case 1:
            inode_reclaim(part, file_ino);
        case 0:
            kfree(io_buf);
            kfree(file_im_ino);
    }
    return err;
}
