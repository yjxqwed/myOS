#include <fs/myfs/dir.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/file.h>
#include <device/ata.h>
#include <mm/kvmm.h>
#include <string.h>
#include <common/debug.h>
#include <common/types.h>

dir_t root_dir;

void open_root_dir(partition_t *part) {
    root_dir.im_inode = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0;
}

dir_t *dir_open(partition_t *part, uint32_t i_no) {
    dir_t *dir = kmalloc(sizeof(dir_t));
    if (dir == NULL) {
        return NULL;
    }
    dir->dir_pos = 0;
    dir->im_inode = inode_open(part, i_no);
    return dir;
}

// num of dir entries per block
#define __nr_des_per_block(de_size) (BLOCK_SIZE / de_size)

int get_dir_entry_by_name(
    const partition_t *part, const dir_t *dir, const char *name,
    dir_entry_t *dir_entry, void *io_buffer
) {
    uint32_t nr_de_per_block = __nr_des_per_block(part->sb->dir_entry_size);
    // handle direct blocks only for now
    for (int i = 0; i < 12; i++) {
        uint32_t lba = dir->im_inode->inode.i_blocks[i];
        if (lba == 0) {
            continue;
        }
        ata_read(part->my_disk, lba, io_buffer, 1);
        dir_entry_t *des = (dir_entry_t *)io_buffer;
        for (int j = 0; j < nr_de_per_block; j++) {
            // a valid dir entry should have a valid inode number
            if (des[j].i_no != 0 && strcmp(name, des[j].filename) == 0) {
                dir_entry = &(des[j]);
                memcpy(&(des[j]), dir_entry, part->sb->dir_entry_size);
                return FSERR_NOERR;
            }
        }
    }
    return -FSERR_NONEXIST;
}

void dir_close(dir_t *dir) {
    if (dir == &root_dir) {
        return;
    }
    inode_close(dir->im_inode);
    kfree(dir);
}

void create_dir_entry(
    const char *filename, uint32_t i_no,
    file_type_e ft, dir_entry_t *de
) {
    ASSERT(i_no != 0);
    ASSERT(strlen(filename) <= MAX_FILE_NAME_LENGTH);
    de->f_type = ft;
    de->i_no = i_no;
    strcpy(filename, de->filename);
}

bool_t write_dir_entry(
    partition_t *part, dir_t *dir, dir_entry_t *dir_entry, void *buf
) {
    // make sure dir is in part
    ASSERT(list_find(part->open_inodes, dir->im_inode->i_tag));

    uint32_t dir_entry_size = part->sb->dir_entry_size;
    uint32_t nr_de_per_block = __nr_des_per_block(dir_entry_size);
    ASSERT(dir->im_inode->inode.i_size % nr_de_per_block == 0);
    for (int i = 0; i < 12; i++) {
        uint32_t lba = dir->im_inode->inode.i_blocks[i];
        if (lba == 0) {
            // no data block, allocate one
            int block_idx = block_alloc(part);
            if (block_idx == -1) {
                // no enough block
                return False;
            }

            block_btmp_sync(part, block_idx);

            uint32_t block_lba = part->start_lba + block_idx;
            memset(buf, 0, BLOCK_SIZE);
            memcpy(dir_entry, buf, dir_entry_size);
            ata_write(part->my_disk, block_lba, buf, 1);
            dir->im_inode->inode.i_blocks[i] = block_lba;
            goto __success__;
        } else {
            // if there is a data block, find a free slot in it
            ata_read(part->my_disk, lba, buf, 1);
            dir_entry_t *des = (dir_entry_t *)buf;
            for (int j = 0; j < nr_de_per_block; j++) {
                if (des[j].i_no == 0) {
                    memcpy(dir_entry, &(des[j]), dir_entry_size);
                    ata_write(part->my_disk, lba, buf, 1);
                    goto __success__;
                }
            }
        }
    }
    // dir is full
    return False;

__success__:
    dir->im_inode->inode.i_size += dir_entry_size;
    dir->im_inode->dirty = True;
    return True;
}
