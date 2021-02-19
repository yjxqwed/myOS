#include <fs/myfs/file.h>
#include <fs/myfs/inode.h>
#include <device/ata.h>
#include <thread/thread.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/dir.h>
#include <fs/myfs/utils.h>
#include <mm/kvmm.h>
#include <string.h>
#include <thread/process.h>
#include <kprintf.h>
#include <common/utils.h>

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

/**
 * @brief install the global fd into task's own fd_table
 * @return private fd
 */
int install_global_fd(int gfd) {
    task_t *task = get_current_thread();
    ASSERT(task->fd_table != NULL);
    for (int i = 3; i < NR_OPEN; i++) {
        if (task->fd_table[i] == -1) {
            task->fd_table[i] = gfd;
            return i;
        }
    }
    return -1;
}

void task_reclaim_fd(int lfd) {
    task_t *task = get_current_thread();
    ASSERT(task->fd_table != NULL);
    task->fd_table[lfd] = -1;
}

int file_create(
    partition_t *part, /* dir_t *pdir, */ im_inode_t *pdir,
    const char *filename, uint32_t flags,
    void *io_buf
) {

    ASSERT(strlen(filename) <= MAX_FILE_NAME_LENGTH);

    int rollback = -1, err = FSERR_NOERR;
    // alloc in memory inode struct
    im_inode_t *file_im_ino = kmalloc(sizeof(im_inode_t));

    if (file_im_ino == NULL) {
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
    if (file_gfd == -1) {
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

    int file_lfd = install_global_fd(file_gfd);
    if (file_lfd == -1) {
        err = -FSERR_NOLOCFD;
        rollback = 2;
        goto __fail__;
    }


    // sync inode bitmap
    inode_btmp_sync(part, file_ino);
    // sync parent inode
    inode_sync(part, pdir, io_buf);
    // sync file inode
    inode_sync(part, file_im_ino, io_buf);
    // add file inode to cache
    __list_push_front(&(part->open_inodes), file_im_ino, i_tag);

    return file_lfd;

__fail__:
    ASSERT(err != FSERR_NOERR);
    ASSERT(rollback == 2 || rollback == 1 || rollback == 0);
    switch (rollback) {
        case 2:
            file_table_reclaim(file_gfd);
        case 1:
            inode_reclaim(part, file_ino);
        case 0:
            kfree(file_im_ino);
    }
    return err;
}

int file_open(partition_t *part, int i_no, uint32_t flags, file_type_e ft) {
    int file_gfd = file_table_get_free_slot();
    if (file_gfd == -1) {
        return -FSERR_NOGLOFD;
    }
    int fd = install_global_fd(file_gfd);
    if (fd == -1) {
        file_table_reclaim(file_gfd);
        return -FSERR_NOLOCFD;
    }

    im_inode_t *file_im = inode_open(part, i_no);
    if (file_im == NULL) {
        file_table_reclaim(file_gfd);
        task_reclaim_fd(fd);
        return -FSERR_NOMEM;
    }
    ASSERT(i_no == file_im->inode.i_no);
    file_table[file_gfd].file_flags = flags;
    file_table[file_gfd].file_pos = 0;
    file_table[file_gfd].im_inode = file_im;
    file_table[file_gfd].file_tp = ft;

    // if write, check write_deny flag
    if (flags & O_WRONLY || flags & O_RDWR) {
        // INT_STATUS old_status = disable_int();
        if (file_im->write_deny) {
            // set_int_status(old_status);
            inode_close(file_im);
            file_table_reclaim(file_gfd);
            task_reclaim_fd(fd);
            return -FSERR_EXCWRITE;
        } else {
            file_im->write_deny = True;
            // set_int_status(old_status);
        }
    }
    return fd;
}


file_t *lfd2file(int local_fd) {
    if (local_fd < 0 || local_fd >= NR_OPEN) {
        return NULL;
    }
    int *fd_table = get_current_thread()->fd_table;
    ASSERT(fd_table != NULL);
    int gfd = fd_table[local_fd];

    if (gfd == -1) {
        return NULL;
    }

    ASSERT(gfd >= 0 && gfd < MAX_FILE_OPEN);

    file_t *file = &(file_table[gfd]);
    ASSERT(file->im_inode != NULL);
    return file;
}


int file_close(int local_fd) {
    // if (local_fd < 0 || local_fd >= NR_OPEN) {
    //     return -FSERR_BADFD;
    // }
    // int *fd_table = get_current_thread()->fd_table;
    // ASSERT(fd_table != NULL);
    // int gfd = fd_table[local_fd];
    // // reclaim local_fd
    // fd_table[local_fd] = -1;

    // if (gfd == -1) {
    //     return -FSERR_BADFD;
    // }

    // ASSERT(gfd >= 0 && gfd < MAX_FILE_OPEN);

    // file_t *file = &(file_table[gfd]);
    // ASSERT(file->im_inode != NULL);
    file_t *file = lfd2file(local_fd);
    if (file == NULL) {
        return -FSERR_BADFD;
    }
    // after passing check of lfd2file, the following line is safe
    get_current_thread()->fd_table[local_fd] = -1;

    inode_close(file->im_inode);
    file->im_inode = NULL;
    return FSERR_NOERR;
}


int file_read(
    partition_t *part, file_t *file, void *buffer, size_t count
) {
    // file must be FT_REGULAR and flags must not contain O_WRONLY
    ASSERT(file->im_inode != NULL && file->file_tp == FT_REGULAR);
    ASSERT(!(file->file_flags & O_WRONLY));

    if (count == 0 || file->file_pos >= file->im_inode->inode.i_size) {
        return 0;
    }

    void *io_buf = kmalloc(BLOCK_SIZE);
    if (io_buf == NULL) {
        return -FSERR_NOMEM;
    }

    count = MIN(count, file->im_inode->inode.i_size - file->file_pos);
    ASSERT(count > 0);
    // we need to read count bytes starting form file_pos
    // and store them in the buffer

    int *blks = file->im_inode->inode.i_blocks;

    int bytes_read = 0;
    while (bytes_read < count) {
        int blk_no = file->file_pos / BLOCK_SIZE;
        ASSERT(blk_no < 12 && blks[blk_no] != 0);
        int blk_off = file->file_pos % BLOCK_SIZE;
        int num_bytes = BLOCK_SIZE - blk_off;
        num_bytes = MIN(num_bytes, count - bytes_read);
        ata_read(part->my_disk, blks[blk_no], io_buf, 1);
        memcpy(io_buf + blk_off, buffer + bytes_read, num_bytes);
        bytes_read += num_bytes;
        file->file_pos += num_bytes;
    }
    ASSERT(bytes_read == count);
    kfree(io_buf);
    ASSERT(file->file_flags <= file->im_inode->inode.i_size);
    return count;
}


int read_dirent(
    partition_t *part, file_t *dfile, void *buffer, size_t count
) {
    ASSERT(dfile->file_tp == FT_DIRECTORY);
    ASSERT(dfile->file_flags == O_RDONLY);
    if (count == 0 || dfile->file_pos >= dfile->im_inode->inode.i_size) {
        return 0;
    }

    int dent_size = part->sb->dir_entry_size;

    int blk_size = BLOCK_SIZE - BLOCK_SIZE % dent_size;

    ASSERT((dfile->im_inode->inode.i_size - dfile->file_pos) % dent_size == 0);

    count = MIN(
        count - count % dent_size, 
        dfile->im_inode->inode.i_size - dfile->file_pos
    );

    ASSERT(count % dent_size == 0);

    kprintf(KPL_DEBUG, "dir_file_read: count = %d\n", count);

    if (count == 0) {
        return 0;
    }

    // we need to read count bytes starting form file_pos
    // and store them in the buffer

    void *io_buf = kmalloc(BLOCK_SIZE);
    if (io_buf == NULL) {
        return -FSERR_NOMEM;
    }

    int *blks = dfile->im_inode->inode.i_blocks;

    uint32_t pos = dfile->file_pos;

    int bytes_read = 0;
    while (bytes_read < count) {
        int blk_no = pos / blk_size;
        // only consider the first 12 direct data blks for now
        ASSERT(blk_no < 12 && blks[blk_no] != 0);
        int blk_off = pos % blk_size;
        int num_bytes = blk_size - blk_off;
        num_bytes = MIN(num_bytes, count - bytes_read);
        ata_read(part->my_disk, blks[blk_no], io_buf, 1);
        memcpy(io_buf + blk_off, buffer + bytes_read, num_bytes);
        bytes_read += num_bytes;
        pos += bytes_read;
    }
    ASSERT(bytes_read == count);

    kfree(io_buf);
    dfile->file_pos += count;
    ASSERT(pos == dfile->file_pos);
    ASSERT(dfile->file_flags <= dfile->im_inode->inode.i_size);
    return count;
}


int file_write(
    partition_t *part, file_t *file, void *buffer, size_t count
) {
    ASSERT(file->im_inode != NULL && file->file_tp == FT_REGULAR);
    ASSERT(file->file_flags & (O_WRONLY | O_RDWR));
    // there are still <free_bytes> free space of this file
    int free_bytes = 12 * BLOCK_SIZE - file->file_pos;
    int err = FSERR_NOERR;
    if (count == 0 || free_bytes == 0) {
        return 0;
    }

    void *io_buf = kmalloc(BLOCK_SIZE);
    if (io_buf == NULL) {
        err = -FSERR_NOMEM;
        return -FSERR_NOMEM;
    }

    int *blks = file->im_inode->inode.i_blocks;
    int bytes_written = 0;
    
    while (bytes_written < count && free_bytes > 0) {
        int blk_no = file->file_pos / BLOCK_SIZE;
        int blk_off = file->file_pos % BLOCK_SIZE;
        // zero-out the io_buffer
        memset(io_buf, 0, BLOCK_SIZE);
        bool_t new_blk = False;
        if (blks[blk_no] == 0) {
            ASSERT(blk_off == 0);
            int blk_idx = block_alloc(part);
            if (blk_idx == -1) {
                err = -FSERR_NOBLOCK;
                break;
            }
            new_blk = True;
            block_btmp_sync(part, blk_idx);
            blks[blk_no] = part->start_lba + blk_idx;
            file->im_inode->dirty = True;
        }
        uint32_t lba = blks[blk_no];
        ASSERT(lba != 0);
        int bytes_to_write = MIN(count - bytes_written, BLOCK_SIZE - blk_off);
        ASSERT(free_bytes >= bytes_to_write);
        if (new_blk) {
            memcpy(buffer, io_buf, bytes_to_write);
        } else {
            ata_read(part->my_disk, lba, io_buf, 1);
            memcpy(buffer, io_buf + blk_off, bytes_to_write);
        }
        ata_write(part->my_disk, lba, io_buf, 1);
        free_bytes -= bytes_to_write;
        bytes_written += bytes_to_write;
        file->file_pos += bytes_to_write;
    }
    ASSERT(bytes_written <= count);
    ASSERT(free_bytes >= 0);
    ASSERT(file->file_pos <= 12 * BLOCK_SIZE);
    // update file's inode i_size if necessary
    if (file->file_pos > file->im_inode->inode.i_size) {
        file->im_inode->inode.i_size = file->file_pos;
        file->im_inode->dirty = True;
    }
    // sync file inode if necessary
    if (file->im_inode->dirty) {
        inode_sync(part, file->im_inode, io_buf);
        file->im_inode->dirty = False;
    }
    kfree(io_buf);
    return bytes_written;
}


int file_truncate(file_t *file, size_t length) {
    ASSERT(file->im_inode != NULL && file->file_tp == FT_REGULAR);
}


/************* some debug utilities *************/

void print_file_table() {
    INT_STATUS old_status = disable_int();
    kprintf(KPL_DEBUG, "\nfile_table: [");
    for (int i = 0; i < MAX_FILE_OPEN; i++) {
        file_t *file = &(file_table[i]);
        if (file->im_inode == NULL) {
            continue;
        }
        kprintf(
            KPL_DEBUG,
            "{(%d)inode={ino=%d,iop=%d,isz=%d},pos=%d,flags=%x,ft=%d}",
            i, file->im_inode->inode.i_no, file->im_inode->i_open_times,
            file->im_inode->inode.i_size,
            file->file_pos, file->file_flags, file->file_tp
        );
    }
    kprintf(KPL_DEBUG, "]\n");
    set_int_status(old_status);
}
