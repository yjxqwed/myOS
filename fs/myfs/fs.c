#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/superblock.h>
#include <fs/myfs/file.h>
#include <fs/myfs/inode.h>
#include <fs/myfs/dir.h>
#include <fs/myfs/utils.h>
#include <myos.h>
#include <device/ata.h>
#include <common/types.h>
#include <common/debug.h>
#include <mm/kvmm.h>
#include <myos.h>
#include <kprintf.h>
#include <list.h>
#include <string.h>
#include <device/tty.h>

#define __super_block_lba(part_start_lba) \
    (part_start_lba + BOOT_BLOCK_SEC_CNT)

extern ata_channel_t channels[2];
extern list_t partition_list;


static int format_partition(partition_t *part) {
    ASSERT(part != NULL);
    kprintf(KPL_NOTICE, "Started to format partition %s\n", part->part_name);
    // boot block occupies 1 sector
    uint32_t boot_block_secs = BOOT_BLOCK_SEC_CNT;
    // super block occupies 1 sector
    uint32_t super_block_secs = SUPER_BLOCK_SEC_CNT;
    super_block_t *sb = (super_block_t *)kmalloc(sizeof(super_block_t));
    if (sb == NULL) {
        return -ERR_MEMORY_SHORTAGE;
    }
    sb->fs_type = FS_MAGIC;
    sb->sec_cnt = part->sec_cnt;
    sb->inode_cnt = MAX_FILE_CNT_PER_PART;
    sb->part_start_lba = part->start_lba;

    uint32_t curr_free_secs_start_lba =
        part->start_lba + boot_block_secs + super_block_secs;

    sb->inode_btmp_start_lba = curr_free_secs_start_lba;
    sb->inode_btmp_sec_cnt = ROUND_UP_DIV(sb->inode_cnt, BLOCK_SIZE_IN_BIT);
    curr_free_secs_start_lba += sb->inode_btmp_sec_cnt;

    sb->inode_table_start_lba = curr_free_secs_start_lba;
    sb->inode_table_sec_cnt =
        ROUND_UP_DIV(sb->inode_cnt, SECTOR_SIZE / sizeof(inode_t));
    curr_free_secs_start_lba += sb->inode_table_sec_cnt;

    sb->block_btmp_start_lba = curr_free_secs_start_lba;
    sb->block_btmp_sec_cnt = ROUND_UP_DIV(sb->sec_cnt, BLOCK_SIZE_IN_BIT);
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
    ata_write(part->my_disk, part->start_lba + boot_block_secs, sb, super_block_secs);
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


static partition_t *curr_part = NULL;


static int mount_partition(const char *part_name) {
    list_node_t *p;
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        if (strcmp(part_name, part->part_name) == 0) {
            // read superblock of this partition
            super_block_t *sb = (super_block_t *)kmalloc(sizeof(super_block_t));
            if (sb == NULL) {
                return -FSERR_NOMEM;
            }
            ata_read(
                part->my_disk, __super_block_lba(part->start_lba),
                sb, SUPER_BLOCK_SEC_CNT
            );
            kprintf(KPL_DEBUG, "sb->root_inode = %d\n", sb->root_inode_no);
            // read inode bitmap of this partition
            part->inode_btmp.bits_ = (uint8_t *)kmalloc(sb->inode_btmp_sec_cnt * SECTOR_SIZE);
            if (part->inode_btmp.bits_ == NULL) {
                kfree(sb);
                return -FSERR_NOMEM;
            }
            ata_read(
                part->my_disk, sb->inode_btmp_start_lba,
                part->inode_btmp.bits_, sb->inode_btmp_sec_cnt
            );
            bitmap_reinit(&(part->inode_btmp), sb->inode_btmp_sec_cnt * SECTOR_SIZE);

            // read block bitmap of this partition
            part->block_btmp.bits_ = (uint8_t *)kmalloc(sb->block_btmp_sec_cnt * SECTOR_SIZE);
            if (part->block_btmp.bits_ == NULL) {
                kfree(sb);
                kfree(part->inode_btmp.bits_);
                return -FSERR_NOMEM;
            }
            ata_read(
                part->my_disk, sb->block_btmp_start_lba,
                part->block_btmp.bits_, sb->block_btmp_sec_cnt
            );
            bitmap_reinit(&(part->block_btmp), sb->block_btmp_sec_cnt * SECTOR_SIZE);

            for (int i = 0; i < NR_DIRTY_BLOCKS; i++) {
                part->dirty_blocks[i].first = 0;
                part->dirty_blocks[i].second = NULL;
            }

            part->sb = sb;
            list_init(&(part->open_inodes));
            curr_part = part;
            kprintf(KPL_NOTICE, "%s mounted!\n", curr_part->part_name);
            print_btmp(&(curr_part->block_btmp));
            print_btmp(&(curr_part->inode_btmp));
            open_root_dir(part);
            return FSERR_NOERR;
        }
    }
    return -FSERR_NOPART;
}


extern im_inode_t *root_imnode;


void fs_init() {
    list_node_t *p;
    super_block_t *sb = kmalloc(sizeof(super_block_t));
    if (sb == NULL) {
        PANIC("failed to init fs");
    }
    __list_for_each((&partition_list), p) {
        partition_t *part = __container_of(partition_t, part_tag, p);
        ata_read(
            part->my_disk, __super_block_lba(part->start_lba),
            sb, SUPER_BLOCK_SEC_CNT
        );
        if (sb->fs_type == FS_MAGIC) {
            kprintf(KPL_NOTICE, "%s has a filesystem\n", part->part_name);
        } else {
            format_partition(part);
        }
        // format_partition(part);
    }
    if (mount_partition("sdc1") != FSERR_NOERR) {
        PANIC("failed to mount sdc1");
    }
    // kprintf(KPL_NOTICE, "root dir (%d) opened.\n", root_dir.im_inode->inode.i_no);
    kprintf(KPL_NOTICE, "root dir (%d) opened.\n", root_imnode->inode.i_no);
    kfree(sb);
    // MAGICBP;
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
                curr_part, curr_part->sb->root_inode_no,
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
    __list_for_each((&(curr_part->open_inodes)), p) {
        im_inode_t *im = __container_of(im_inode_t, i_tag, p);
        kprintf(
            KPL_DEBUG, "{ino=%d, iop=%d}",
            im->inode.i_no, im->i_open_times
        );
    }
    kprintf(KPL_DEBUG, "]\n");
    set_int_status(old_status);
}
