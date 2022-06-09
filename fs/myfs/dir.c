#include <fs/myfs/dir.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/file.h>
#include <fs/myfs/utils.h>
#include <device/ata.h>
#include <mm/kvmm.h>
#include <string.h>
#include <common/debug.h>
#include <common/types.h>
#include <kprintf.h>

// dir_t root_dir;

im_inode_t *root_imnode = NULL;

void open_root_dir(partition_t *part) {
    // root_dir.im_inode = inode_open(part, part->sb->root_inode_no);
    // root_imnode = inode_open(part, part->sb->root_inode_no);
    root_imnode = inode_open(part, __myfs_field(part, sb)->root_inode_no);
    // root_dir.dir_pos = 0;
}

im_inode_t *dir_open(partition_t *part, uint32_t i_no) {
    return inode_open(part, i_no);
}

// num of dir-entries per block
#define __nr_des_per_block(de_size) (BLOCK_SIZE / de_size)

int get_dir_entry_by_name(
    const partition_t *part, const im_inode_t *dir,
    // const dir_t *dir, 
    const char *name, dir_entry_t *dir_entry, void *io_buffer
) {
    uint32_t nr_de_per_block = __nr_des_per_block(__myfs_field(part, sb)->dir_entry_size);
    // handle direct blocks only for now
    for (int i = 0; i < 12; i++) {
        // uint32_t lba = dir->im_inode->inode.i_blocks[i];
        uint32_t lba = dir->inode.i_blocks[i];
        if (lba == 0) {
            continue;
        }
        ata_read(part->my_disk, lba, io_buffer, 1);
        dir_entry_t *des = (dir_entry_t *)io_buffer;
        for (int j = 0; j < nr_de_per_block; j++) {
            // a valid dir entry should have a valid inode number
            if (des[j].f_type != FT_NONE && !strcmp(name, des[j].filename)) {
                // dir_entry = &(des[j]);
                memcpy(&(des[j]), dir_entry, __myfs_field(part, sb)->dir_entry_size);
                return FSERR_NOERR;
            }
        }
    }
    return -FSERR_NONEXIST;
}

void dir_close(im_inode_t *dir) {
    if (dir == root_imnode) {
        return;
    }
    inode_close(dir);
}

void create_dir_entry(
    const char *filename, uint32_t i_no, file_type_e ft, dir_entry_t *de
) {
    de->f_type = ft;
    de->i_no = i_no;
    strcpy(filename, de->filename);
}

int write_dir_entry(
    partition_t *part, /* dir_t *dir, */ im_inode_t *dir,
    dir_entry_t *dir_entry, void *buf
) {
    // make sure dir is in part
    // ASSERT(list_find(&(part->open_inodes), &(dir->im_inode->i_tag)));
    ASSERT(list_find(&__myfs_field(part, open_inodes), &(dir->i_tag)));

    uint32_t dir_entry_size = __myfs_field(part, sb)->dir_entry_size;
    uint32_t nr_de_per_block = __nr_des_per_block(dir_entry_size);
    // ASSERT(dir->im_inode->inode.i_size % nr_de_per_block == 0);
    ASSERT(dir->inode.i_size % nr_de_per_block == 0);

    // only use direct blocks for now
    for (int i = 0; i < 12; i++) {
        // uint32_t lba = dir->im_inode->inode.i_blocks[i];
        uint32_t lba = dir->inode.i_blocks[i];
        if (lba == 0) {
            // no data block, allocate one
            int block_idx = block_alloc(part);
            if (block_idx == -1) {
                // no enough block
                return -FSERR_NOBLOCK;
            }

            block_btmp_sync(part, block_idx);

            uint32_t block_lba = part->start_lba + block_idx;
            memset(buf, 0, BLOCK_SIZE);
            memcpy(dir_entry, buf, dir_entry_size);
            ata_write(part->my_disk, block_lba, buf, 1);
            // dir->im_inode->inode.i_blocks[i] = block_lba;
            dir->inode.i_blocks[i] = block_lba;
            goto __success__;
        } else {
            // if there is a data block, find a free slot in it
            ata_read(part->my_disk, lba, buf, 1);
            dir_entry_t *des = (dir_entry_t *)buf;
            for (int j = 0; j < nr_de_per_block; j++) {
                if (des[j].f_type == FT_NONE) {
                    memcpy(dir_entry, &(des[j]), dir_entry_size);
                    ata_write(part->my_disk, lba, buf, 1);
                    goto __success__;
                }
            }
        }
    }
    // dir is full
    return -FSERR_DIRFULL;

__success__:
    // dir->im_inode->inode.i_size += dir_entry_size;
    // dir->im_inode->dirty = True;
    dir->inode.i_size += dir_entry_size;
    return FSERR_NOERR;
}


int dir_create(
    partition_t *part, im_inode_t *pdir, const char *dirname,
    void *io_buf
) {
    ASSERT(strlen(dirname) <= MAX_FILE_NAME_LENGTH);

    // alloc an inode no
    int dir_ino = inode_alloc(part);
    if (dir_ino == -1) {
        return -FSERR_NOINODE;
    }

    // alloc a data block since an empty dir has 2 entries.
    int blk_idx = block_alloc(part);
    if (blk_idx == -1) {
        inode_reclaim(part, dir_ino);
        return -FSERR_NOBLOCK;
    }

    // write dir entry under pdir
    dir_entry_t dentry;
    create_dir_entry(dirname, dir_ino, FT_DIRECTORY, &dentry);
    int err = write_dir_entry(part, pdir, &dentry, io_buf);
    if (err != FSERR_NOERR) {
        inode_reclaim(part, dir_ino);
        block_reclaim(part, blk_idx);
        return err;
    }

    // init the data block
    int data_lba = part->start_lba + blk_idx;
    memset(io_buf, 0, BLOCK_SIZE);
    dir_entry_t *de = (dir_entry_t *)io_buf;
    // init .
    strcpy(".", de[0].filename);
    de[0].i_no = dir_ino;
    de[0].f_type = FT_DIRECTORY;
    // init ..
    strcpy("..", de[1].filename);
    de[1].i_no = pdir->inode.i_no;
    de[1].f_type = FT_DIRECTORY;
    ata_write(part->my_disk, data_lba, de, 1);

    // inode struct for dir
    im_inode_t dir_im_ino;
    im_inode_init(&dir_im_ino, dir_ino);
    dir_im_ino.inode.i_blocks[0] = data_lba;
    dir_im_ino.inode.i_size = sizeof(dir_entry_t) * 2;

    // sync the block btmp for blk_idx being used
    block_btmp_sync(part, blk_idx);
    // sync inode btmp for dir_ino being used
    inode_btmp_sync(part, dir_ino);
    // sync dir inode
    inode_sync(part, &dir_im_ino, io_buf);
    // sync pdir inode (size changed in write_dir_entry)
    inode_sync(part, pdir, io_buf);

    return FSERR_NOERR;
}


void print_dentry(const dir_entry_t *dent) {
    kprintf(
        KPL_DEBUG, "{filename=%s, ino=%d, ft=%s}\n",
        dent->filename, dent->i_no,
        dent->f_type == FT_DIRECTORY ? "dir" : "reg"
    );
}
