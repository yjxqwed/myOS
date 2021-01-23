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
    // inode_sync(part, pdir->im_inode, io_buf);
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


int file_read(partition_t *part, int local_fd, void *buffer, size_t count) {

    file_t *file = lfd2file(local_fd);
    if (file == NULL) {
        return -FSERR_BADFD;
    }

    if (count == 0 || file->file_pos >= file->im_inode->inode.i_size) {
        return 0;
    }

    void *io_buf = kmalloc(BLOCK_SIZE);
    if (io_buf == NULL) {
        return -FSERR_NOMEM;
    }

    int blk_size = BLOCK_SIZE;
    int dentry_size = part->sb->dir_entry_size;

    if (file->file_tp == FT_DIRECTORY) {
        // for dir file, each block may not be fully used
        blk_size -= BLOCK_SIZE % dentry_size;
    }

    int bytes_read = 0;
    int bytes_read_per_blk = 0;

    int *blocks = file->im_inode->inode.i_blocks;
    // number of dentries per block
    int ndpb = BLOCK_SIZE / dentry_size;

    int start_blk_no = file->file_pos / blk_size;
    int start_blk_off = file->file_pos % blk_size;

    // count is the number of bytes of dentries to be read
    count = MIN(count, file->im_inode->inode.i_size - file->file_pos);
    count -= count % dentry_size;

    int num_blks = ROUND_UP_DIV(count + start_blk_off * dentry_size, blk_size);

    // only consider the first 12 direct blocks
    ASSERT(start_blk_no + num_blks <= 12);

    kfree(io_buf);
}


int dir_file_read(
    partition_t *part, file_t *dfile, void *buffer, size_t count
) {
    if (count == 0 || dfile->file_pos >= dfile->im_inode->inode.i_size) {
        return 0;
    }
    dir_entry_t *dentries_buf = (dir_entry_t *)buffer;
    int dent_size = part->sb->dir_entry_size;
    int nr_dents = count / dent_size;
    // the buffer can hold at most nr_dents entries

    return -1;
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
            "{(%d)inode={ino=%d,iop=%d},pos=%d,flags=%x,ft=%d}",
            i, file->im_inode->inode.i_no, file->im_inode->i_open_times,
            file->file_pos, file->file_flags, file->file_tp
        );
    }
    kprintf(KPL_DEBUG, "]\n");
    set_int_status(old_status);
}
