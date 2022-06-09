#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/file.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/dir.h>
#include <fs/myfs/utils.h>
#include <myos.h>
#include <device/ata.h>
#include <common/types.h>
#include <common/debug.h>
#include <mm/kvmm.h>
#include <lib/kprintf.h>
#include <lib/list.h>
#include <lib/string.h>
#include <device/tty.h>

extern list_t partition_list;

static int format_partition(partition_t *part, uint8_t *buf, uint32_t buf_size) {
    ASSERT(part != NULL);
    kprintf(KPL_NOTICE, "Started to format partition %s\n", part->part_name);

    ASSERT(buf_size == SUPER_BLOCK_BLK_CNT * BLOCK_SIZE)
    // uint8_t *buf = (uint8_t *)kmalloc(buf_size);
    // if (buf == NULL) {
    //     kfree(buf);
    //     return -FSERR_NOMEM;
    // }

    super_block_t *sb = (super_block_t *)buf;

    sb->fs_type = FS_MAGIC;
    sb->block_cnt = part->sec_cnt / NR_SECTORS_PER_BLOCK;
    sb->inode_cnt = MAX_FILE_CNT_PER_PART;
    sb->part_start_lba = part->start_lba;

    uint32_t curr_free_blks_start_id = BOOT_BLOCK_BLK_CNT + SUPER_BLOCK_BLK_CNT;
    // inode_btmp
    sb->inode_btmp_start_blk_id = curr_free_blks_start_id;
    sb->inode_btmp_blk_cnt = ROUND_UP_DIV(sb->inode_cnt, BLOCK_SIZE_IN_BIT);
    curr_free_blks_start_id += sb->inode_btmp_blk_cnt;
    // inode_table
    sb->inode_table_start_blk_id = curr_free_blks_start_id;
    sb->inode_table_blk_cnt = ROUND_UP_DIV(sb->inode_cnt, BLOCK_SIZE / sizeof(inode_t));
    curr_free_blks_start_id += sb->inode_table_blk_cnt;
    // block_btmp
    sb->block_btmp_start_blk_id = curr_free_blks_start_id;
    sb->block_btmp_blk_cnt = ROUND_UP_DIV(sb->block_cnt, BLOCK_SIZE_IN_BIT);
    curr_free_blks_start_id += sb->block_btmp_blk_cnt;

    sb->data_start_blk_id = curr_free_blks_start_id;

    sb->root_inode_no = 0;
    sb->dir_entry_size = sizeof(dir_entry_t);

    kprintf(KPL_DEBUG, "%s info: \n", part->part_name);
    kprintf(
        KPL_DEBUG,
        "    fs_type: 0x%x\n"
        "    part_start_lba: 0x%x\n"
        "    nr_sectors_per_block: 0x%x\n"
        "    blk_cnt: 0x%x\n"
        "    inode_cnt: 0x%x\n"
        "    inode_btmp_start_blk_id: 0x%x\n"
        "    inode_btmp_blk_cnt: 0x%x\n"
        "    inode_table_start_blk_id: 0x%x\n"
        "    inode_table_blk_cnt: 0x%x\n"
        "    block_btmp_start_blk_id: 0x%x\n"
        "    block_btmp_blk_cnt: 0x%x\n"
        "    data_start_blk_id: 0x%x\n",
        sb->fs_type, sb->part_start_lba, NR_SECTORS_PER_BLOCK,
        sb->block_cnt, sb->inode_cnt,
        sb->inode_btmp_start_blk_id, sb->inode_btmp_blk_cnt,
        sb->inode_table_start_blk_id, sb->inode_table_blk_cnt,
        sb->block_btmp_start_blk_id, sb->block_btmp_blk_cnt,
        sb->data_start_blk_id
    );

    // write super block to disk
    kprintf(KPL_NOTICE, "  Writing super block to disk... ");
    partition_block_write(part, BOOT_BLOCK_BLK_CNT, sb, SUPER_BLOCK_BLK_CNT);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, sizeof(super_block_t));

    // init inode_btmp
    buf[0] = 0x01;  // inode 0 has been assigned to root
    kprintf(KPL_NOTICE, "  Writing inode bitmap to disk... ");
    partition_block_write(part, sb->inode_btmp_start_blk_id, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");
    buf[0] = 0;

    // init inode_table
    inode_t *inode = (inode_t *)buf;
    inode->i_no = 0;  // the first inode is for root
    // root is a dir and it contains . (self) and .. (parent dir) by default as its dir entries
    inode->i_size = sb->dir_entry_size * 2;
    inode->i_blocks[0] = sb->data_start_blk_id;
    kprintf(KPL_NOTICE, "  Writing inode table to disk... ");
    partition_block_write(part, sb->inode_table_start_blk_id, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");
    memset(buf, 0, sizeof(inode_t));

    /** init block_btmp */
    // size of the btmp in bytes
    uint32_t byte_len = sb->block_btmp_blk_cnt * BLOCK_SIZE;
    // size of the btmp in bits
    uint32_t bit_len = byte_len * 8;

    // the used-block-bits and the none-block-bits should be 1
    // others should be 0
    kprintf(KPL_NOTICE, "  Writing block bitmap to disk... ");
    uint32_t btmp_blk_offset = 0;
    for (uint32_t i = 0; i <= sb->data_start_blk_id; i++) {
        uint32_t byte_offset = i / 8 - btmp_blk_offset * BLOCK_SIZE, bit_offset = i % 8;
        if (byte_offset >= BLOCK_SIZE) {
            uint32_t bid = sb->block_btmp_start_blk_id + btmp_blk_offset;
            partition_block_write(part, bid, buf, 1);
            memset(buf, 0, BLOCK_SIZE);
            btmp_blk_offset++;
            byte_offset -= BLOCK_SIZE;
        }
        buf[byte_offset] |= (1 << bit_offset);
    }

    ASSERT(btmp_blk_offset <= sb->block_cnt / BLOCK_SIZE_IN_BIT);
    if (btmp_blk_offset < sb->block_cnt / BLOCK_SIZE_IN_BIT) {
        uint32_t bid = sb->block_btmp_start_blk_id + btmp_blk_offset;
        partition_block_write(part, bid, buf, 1);
        memset(buf, 0, BLOCK_SIZE);
    }

    btmp_blk_offset = sb->block_cnt / BLOCK_SIZE_IN_BIT;
    for (uint32_t i = sb->block_cnt; i < bit_len; i++) {
        uint32_t byte_offset = i / 8 - btmp_blk_offset * BLOCK_SIZE, bit_offset = i % 8;
        buf[byte_offset] |= (1 << bit_offset);
    }
    // always write the last block
    uint32_t bid = sb->block_btmp_start_blk_id + sb->block_btmp_blk_cnt - 1;
    partition_block_write(part, bid, buf, 1);
    memset(buf, 0, BLOCK_SIZE);
    kprintf(KPL_NOTICE, "  Done!\n");

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
    partition_block_write(part, sb->data_start_blk_id, buf, 1);
    kprintf(KPL_NOTICE, "  Done!\n");

    kprintf(KPL_NOTICE, "Done formatting partition %s\n", part->part_name);
    return FSERR_NOERR;
}


static partition_t *curr_part = NULL;


static int mount_partition(partition_t *part) {
    if (part == NULL) {
        return -FSERR_NOPART;
    }

    // read the superblock of this partition from disk
    uint32_t buf_size = SUPER_BLOCK_BLK_CNT * BLOCK_SIZE;
    uint8_t *buf = (uint8_t *)kmalloc(buf_size);
    myfs_struct_t *myfs = (myfs_struct_t *)kmalloc(sizeof(myfs_struct_t));
    super_block_t *sb = (super_block_t *)kmalloc(sizeof(super_block_t));
    if (buf == NULL || myfs == NULL || sb == NULL) {
        kfree(buf);
        kfree(myfs);
        kfree(sb);
        return -FSERR_NOMEM;
    }
    partition_block_read(part, SUPER_BLOCK_START_BLK_ID, buf, SUPER_BLOCK_BLK_CNT);
    memcpy(buf, sb, sizeof(super_block_t));
    kfree(buf);
    kprintf(KPL_DEBUG, "sb->root_inode = %d\n", sb->root_inode_no);

    uint32_t inode_btmp_byte_len = sb->inode_btmp_blk_cnt * BLOCK_SIZE;
    uint32_t block_btmp_byte_len = sb->block_btmp_blk_cnt * BLOCK_SIZE;
    myfs->inode_btmp.bits_ = (uint8_t *)kmalloc(inode_btmp_byte_len);
    myfs->block_btmp.bits_ = (uint8_t *)kmalloc(block_btmp_byte_len);
    if (myfs->inode_btmp.bits_ == NULL || myfs->block_btmp.bits_ == NULL) {
        kfree(myfs);
        kfree(sb);
        kfree(myfs->inode_btmp.bits_);
        kfree(myfs->block_btmp.bits_);
        return -FSERR_NOMEM;
    }
    partition_block_read(part, sb->inode_btmp_start_blk_id, myfs->inode_btmp.bits_, sb->inode_btmp_blk_cnt);
    bitmap_reinit(&(myfs->inode_btmp), inode_btmp_byte_len);

    partition_block_read(part, sb->block_btmp_start_blk_id, myfs->block_btmp.bits_, sb->block_btmp_blk_cnt);
    bitmap_reinit(&(myfs->block_btmp), block_btmp_byte_len);

    // for (int i = 0; i < NR_DIRTY_BLOCKS; i++) {
    //     myfs->dirty_blocks[i].first = 0;
    //     myfs->dirty_blocks[i].second = NULL;
    // }

    myfs->sb = sb;
    list_init(&(myfs->open_inodes));
    part->fs_struct = myfs;
    curr_part = part;
    kprintf(KPL_NOTICE, "%s mounted!\n", curr_part->part_name);
    print_btmp(&__myfs_field(curr_part, block_btmp));
    print_btmp(&__myfs_field(curr_part, inode_btmp));
    open_root_dir(part);
    return FSERR_NOERR;
}


extern im_inode_t *root_imnode;


void fs_init() {
    uint32_t buf_size = SUPER_BLOCK_BLK_CNT * BLOCK_SIZE;
    uint8_t *buf = (uint8_t *)kmalloc(buf_size);
    if (buf == NULL) {
        PANIC("failed to init fs [NOMEM]");
    }
    super_block_t *sb = (super_block_t *)buf;
    partition_t *first_part = NULL;

    list_node_t *p;
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        if (first_part == NULL) {
            first_part = part;
        }
        partition_block_read(part, SUPER_BLOCK_START_BLK_ID, sb, SUPER_BLOCK_BLK_CNT);
        if (sb->fs_type == FS_MAGIC) {
            kprintf(KPL_NOTICE, "%s has a filesystem\n", part->part_name);
        } else {
            format_partition(part, buf, buf_size);
        }
    }

    // we use the first partition as the root directory :-)
    if (!first_part || mount_partition(first_part) != FSERR_NOERR) {
        PANIC("failed to mount the first partition");
    }
    // kprintf(KPL_NOTICE, "root dir (%d) opened.\n", root_imnode->inode.i_no);
    kfree(buf);
}


/**
 * @brief get the parent dir of a path
 * @example ./a/b/c will get b
 */
int get_pdir(
    partition_t *part, path_info_t *pi, void *io_buffer,
    // dir_t **pdir
    im_inode_t **pdir
) {
    // dir_t *cdir = pi->abs ? &root_dir : get_current_thread()->cwd_dir;
    im_inode_t *cdir = root_imnode;
    if (!(pi->abs)) {
        cdir = dir_open(part, get_current_thread()->cwd_inode_no);
    }
    ASSERT(cdir != NULL);
    ASSERT(pi->depth != 0);
    dir_entry_t dentry;
    int err = FSERR_NOERR;
    for (int i = 0; i < pi->depth - 1; i++) {
        err = get_dir_entry_by_name(
            part, cdir, pi->path[i], &dentry, io_buffer
        );
        dir_close(cdir);
        if (err != FSERR_NOERR) {
            return err;
        }
        if (dentry.f_type != FT_DIRECTORY) {
            return -FSERR_NOTDIR;
        }
        cdir = dir_open(part, dentry.i_no);
    }
    *pdir = cdir;
    return FSERR_NOERR;
}


int sys_open(const char *pathname, uint32_t flags) {
    int fd = -1;
    path_info_t *pi = (path_info_t *)kmalloc(sizeof(path_info_t));
    if (pi == NULL) {
        return -FSERR_NOMEM;
    }
    int err = analyze_path(pathname, pi);
    if (err != FSERR_NOERR) {
        kfree(pi);
        return err;
    }

    if (pi->isdir && flags != O_RDONLY) {
        // dir can only be read
        kfree(pi);
        return -FSERR_DIRECTORY;
    }

    if (pi->depth == 0) {
        if (pi->abs) {
            // '/' -> root dir
            fd = file_open(
                curr_part, __myfs_field(curr_part, sb)->root_inode_no,
                flags, FT_DIRECTORY
            );
        } else {
            // '.' -> this dir
            fd = file_open(
                curr_part, get_current_thread()->cwd_inode_no,
                flags, FT_DIRECTORY
            );
        }
        err = FSERR_NOERR;
        goto __success__;
    }

    void *io_buffer = kmalloc(BLOCK_SIZE);
    if (io_buffer == NULL) {
        kfree(pi);
        return -FSERR_NOMEM;
    }

    im_inode_t *pdir = NULL;
    err = get_pdir(curr_part, pi, io_buffer, &pdir);
    if (err != FSERR_NOERR) {
        kfree(pi);
        kfree(io_buffer);
        return err;
    }

    kprintf(KPL_DEBUG, "pdir.inode = %d\n", pdir->inode.i_no);

    dir_entry_t dir_ent;
    char *target_file = pi->path[pi->depth - 1];
    kprintf(KPL_DEBUG, "target_file: %s\n", target_file);
    err = get_dir_entry_by_name(
        curr_part, pdir, target_file, &dir_ent, io_buffer
    );

    kprintf(
        KPL_DEBUG,
        "dir_ent: i_no = %d, ft = %d, filename = %s\n",
        dir_ent.i_no, dir_ent.f_type, dir_ent.filename
    );

    if (err == FSERR_NOERR) {
        // found an dentry
        if (pi->isdir && dir_ent.f_type != FT_DIRECTORY) {
            // looking for a dir but found a not dir
            err = -FSERR_NOTDIR;
            goto __success__;
        }

        pi->isdir == (dir_ent.f_type == FT_DIRECTORY);

        if (pi->isdir) {
            if (flags == O_RDONLY) {
                err = FSERR_NOERR;
                fd = file_open(curr_part, dir_ent.i_no, flags, dir_ent.f_type);
                goto __success__;
            } else {
                err = -FSERR_DIRECTORY;
                goto __success__;
            }
        } else {
            if (flags & O_CREAT) {
                err = -FSERR_EXIST;
                goto __success__;
            } else {
                err = FSERR_NOERR;
                fd = file_open(curr_part, dir_ent.i_no, flags, dir_ent.f_type);
                goto __success__;
            }
        }
    } else {
        // not found (err == -FSERR_NONEXIST)
        if (flags & O_CREAT) {
            ASSERT(!pi->isdir);
            err = FSERR_NOERR;
            fd = file_create(curr_part, pdir, target_file, flags, io_buffer);
            goto __success__;
        } else {
            err = -FSERR_NONEXIST;
            goto __success__;
        }
    }

__success__:
    kfree(pi);
    kfree(io_buffer);
    return (err == FSERR_NOERR) ? fd : err;
}


int sys_close(int fd) {
    return file_close(fd);
}


int sys_getdents(int fd, void *buffer, size_t count) {
    file_t *file = lfd2file(fd);
    if (
        file == NULL ||
        file->file_tp != FT_DIRECTORY ||
        file->file_flags != O_RDONLY
    ) {
        return -FSERR_BADFD;
    }
    return read_dirent(curr_part, file, buffer, count);
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
    file_t *file = lfd2file(fd);
    if (
        file == NULL ||
        file->file_tp != FT_REGULAR ||
        file->file_flags & O_WRONLY
    ) {
        return -FSERR_BADFD;
    }
    return file_read(curr_part, file, buffer, count);
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
    file_t *file = lfd2file(fd);
    if (
        file == NULL ||
        file->file_tp != FT_REGULAR ||
        !(file->file_flags & (O_WRONLY | O_RDWR))
    ) {
        return -FSERR_BADFD;
    }
    return file_write(curr_part, file, buffer, count);
}


off_t sys_lseek(int fd, off_t offset, int whence) {
    file_t *file = lfd2file(fd);
    if (file == NULL || file->file_tp != FT_REGULAR) {
        return -FSERR_BADFD;
    }

    int pos = -1, file_sz = file->im_inode->inode.i_size;
    if (whence == SEEK_SET) {
        pos = offset;
    } else if (whence == SEEK_CUR) {
        pos = file->file_pos + offset;
    } else if (whence == SEEK_END) {
        pos = file_sz + offset;
    } else {
        return -FSERR_BADWHENCE;
    }

    if (pos >= 0 && pos < file_sz) {
        file->file_pos = pos;
        return pos;
    } else {
        return -FSERR_BADOFF;
    }
}

int sys_rewinddir(int fd) {
    file_t *file = lfd2file(fd);
    if (file == NULL || file->file_tp != FT_DIRECTORY) {
        return -FSERR_BADFD;
    }
    file->file_pos = 0;
    return 0;
}


int sys_mkdir(const char *pathname) {
    path_info_t *pi = (path_info_t *)kmalloc(sizeof(path_info_t));
    if (pi == NULL) {
        return -FSERR_NOMEM;
    }
    int err = analyze_path(pathname, pi);
    if (err != FSERR_NOERR) {
        kfree(pi);
        return err;
    }
    if (pi->depth == 0) {
        kfree(pi);
        // depth = 0 is . or /, must exist
        return -FSERR_EXIST;
    }
    void *io_buffer = kmalloc(BLOCK_SIZE);
    if (io_buffer == NULL) {
        kfree(pi);
        return -FSERR_NOMEM;
    }

    im_inode_t *pdir = NULL;
    err = get_pdir(curr_part, pi, io_buffer, &pdir);
    if (err != FSERR_NOERR) {
        kfree(pi);
        kfree(io_buffer);
        return err;
    }

    dir_entry_t dentry;
    char *dir_name = pi->path[pi->depth - 1];
    err = get_dir_entry_by_name(curr_part, pdir, dir_name, &dentry, io_buffer);
    if (err == FSERR_NOERR) {
        // file already exists
        kfree(pi);
        kfree(io_buffer);
        return -FSERR_EXIST;
    }

    err = dir_create(curr_part, pdir, dir_name, io_buffer);
    kfree(pi);
    kfree(io_buffer);
    return err;
}

/**
 * @brief get parent dir's (..) inode number
 * @param part current partition
 * @param inode_nr inode number of me
 * @return >= 0 if success; < 0 if failure
 */
static int get_pdir_inode_nr(partition_t *part, int inode_nr) {

    return 0;
}

int sys_getcwd(char *buf, size_t size) {
    task_t *t = get_current_thread();
    int cwd_inode_no = t->cwd_inode_no;
    kprintf(KPL_DEBUG, "cwd: %d\n", cwd_inode_no);
    if (cwd_inode_no == 0) {
        if (size >= 2) {
            strcpy("/", buf);
            return FSERR_NOERR;
        } else {
            return -FSERR_RANGE;
        }
    }

    typedef char file_name_t[MAX_FILE_NAME_LENGTH];
    file_name_t *path = (file_name_t *)kmalloc(MAX_PATH_DEPTH);
    if (!path) {
        // unable to allocate temp memory
        return -FSERR_NOMEM;
    }
    void *io_buf = kmalloc(BLOCK_SIZE);
    if (!io_buf) {
        // unable to allocate temp memory
        kfree(path);
        return -FSERR_NOMEM;
    }
    int depth = 0;
    // iterate to find root
    while (cwd_inode_no != 0) {
        // im_inode_t *pdir = dir_open(curr_part, )
    }

    kfree(path);
    return 0;
}


int sys_chdir(const char *path) {
    return 0;
}


/************* some debug utilities *************/

void print_fd_table() {
    task_t *task = get_current_thread();
    INT_STATUS old_status = disable_int();
    int *fd_table = task->fd_table;
    ASSERT(fd_table != NULL);
    kprintf(KPL_DEBUG, "\nfd_table: [%d", fd_table[0]);
    for (int i = 1; i < NR_OPEN; i++) {
        kprintf(KPL_DEBUG, ",%d", fd_table[i]);
    }
    kprintf(KPL_DEBUG, "]\n");
    set_int_status(old_status);
}

void print_open_inodes() {
    INT_STATUS old_status = disable_int();
    list_node_t *p;
    kprintf(KPL_DEBUG, "\n%s open_inodes: [", curr_part->part_name);
    list_t *part_open_inodes = &__myfs_field(curr_part, open_inodes);
    __list_for_each(part_open_inodes, p) {
        im_inode_t *im = __container_of(im_inode_t, i_tag, p);
        kprintf(
            KPL_DEBUG, "{ino=%d, iop=%d}",
            im->inode.i_no, im->i_open_times
        );
    }
    kprintf(KPL_DEBUG, "]\n");
    set_int_status(old_status);
}
