#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>
#include <fs/myfs/superblock.h>
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
                return -ERR_MEMORY_SHORTAGE;
            }
            ata_read(
                part->my_disk, __super_block_lba(part->start_lba),
                sb, SUPER_BLOCK_SEC_CNT
            );

            // read inode bitmap of this partition
            part->inode_btmp.bits_ = (uint8_t *)kmalloc(sb->inode_btmp_sec_cnt * SECTOR_SIZE);
            if (part->inode_btmp.bits_ == NULL) {
                kfree(sb);
                return -ERR_MEMORY_SHORTAGE;
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
                return -ERR_MEMORY_SHORTAGE;
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
            return FSERR_NOERR;
        }
    }
    return -FSERR_NOPART;
}

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
    kfree(sb);
    // MAGICBP;
}

// static char *parse_path(
//     char *pathname, char name_store[MAX_FILE_NAME_LENGTH + 1]
// ) {
//     // ASSERT(pathname != NULL);
//     ASSERT(__valid_kva(pathname));
//     ASSERT(__valid_kva(name_store));

//     if (pathname[0] == '\0') {
//         return NULL;
//     }

//     // skip / s
//     while (*pathname == '/') {
//         pathname++;
//     }

//     while (*pathname != '/' && *pathname != '\0') {
//         *name_store = *pathname;
//         name_store++;
//         pathname++;
//     }
//     *name_store = '\0';

//     while (*pathname == '/') {
//         pathname++;
//     }

//     return pathname;
// }


// /**
//  * @brief depth of a path; "/a/b/c"'s depth is 3
//  */
// uint32_t path_depth(char* pathname) {
//     ASSERT(__valid_kva(pathname));
//     char name[MAX_FILE_NAME_LENGTH + 1];
//     uint32_t depth = 0;

//     pathname = parse_path(pathname, name);
//     while (pathname) {
//         depth++;
//         pathname = parse_path(pathname, name);
//     }

//     return depth;
// }

/**
 * search result
 */
// typedef struct {
//     // info about the last found file
//     char filename[MAX_FILE_NAME_LENGTH + 1];
//     file_type_e ftype;
//     // parent dir of the last found file
//     dir_t *pdir;
// } search_result_t;


extern dir_t root_dir;

/**
 * 找文件, 如果找到, 返回该文件 inode no, 父目录 dir,
 * 如果找不到, 返回第一个不存在文件的父目录 dir, 到第一个不存在文件的路径.
 */
// static int search_file(
//     const partition_t *part, const dir_t *current_dir,
//     path_info_t *pi, void *io_buffer
// ) {

//     const dir_t *cdir = pi->abs ? &root_dir : current_dir;
//     if (pi->depth == 0) {
//         pi->valid_depth = 0;
//         return cdir->im_inode->inode.i_no;
//     }
//     dir_entry_t dir_entry;

//     pathname = parse_path(pathname, filename);
//     while (filename[0] != '\0') {
//         if (filename[0] == '.' && filename[1] == '\0') {
//             pathname = parse_path(pathname, filename);
//             continue;
//         }
//         int res = get_dir_entry_by_name(
//             part, cdir, filename, &dir_entry, io_buffer
//         );
//         if (res == FSERR_NOERR) {
//             // found
//             ASSERT(strcmp(filename, dir_entry.filename) == 0);
//             if (pathname[0] == '\0') {
//                 // found target
//                 return dir_entry.i_no;
//             } else {
//                 // not target
//                 if (dir_entry.f_type == FT_DIRECTORY) {
//                     // is dir
//                     dir_close(cdir);
//                     cdir = dir_open(part, dir_entry.i_no);
//                     pathname = parse_path(pathname, filename);
//                     ASSERT(filename[0] != '\0');
//                     continue;
//                 } else {
//                     // not dir
//                     return -FSERR_NOTDIR;
//                 }
//             }
//         } else {
//             // not found
//             return -FSERR_NONEXIST;
//         }
//     }
// }

int get_pdir(partition_t *part, path_info_t *pi, void *io_buffer, dir_t **pdir) {
    return NULL;
}

int sys_open(partition_t *part, const char *pathname, uint32_t flags) {
    int path_len = strlen(pathname);
    if (path_len > MAX_PATH_LENGTH) {
        return -FSERR_PATHTOOLONG;
    } else if (pathname[path_len - 1] == '/') {
        return -FSERR_DIRECTORY;
    }
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
    if (pi->depth == 0) {
        // if depth is 0, the path is '/' or '.', which are both directories
        kfree(pi);
        return -FSERR_DIRECTORY;
    }
    if (strcmp(pi->path[pi->depth - 1].filename, "..") == 0) {
        // if the target is .., it is a directory
        kfree(pi);
        return -FSERR_DIRECTORY;
    }
    void *io_buffer = kmalloc(BLOCK_SIZE);
    if (io_buffer == NULL) {
        kfree(pi);
        return -FSERR_NOMEM;
    }
    dir_t *pdir = NULL;
    err = get_pdir(part, pi, io_buffer, &pdir);
    if (err != FSERR_NOERR) {
        kfree(pi);
        kfree(io_buffer);
        return err;
    }
    dir_entry_t dir_ent;
    err = get_dir_entry_by_name(part, pdir, pi->path[pi->depth - 1].filename, &dir_ent, io_buffer);
    if (flags & O_CREAT) {
        if (err == FSERR_NOERR) {
            kfree(pi);
            kfree(io_buffer);
        } else {
            kfree(pi);
            kfree(io_buffer);
            return -FSERR_EXIST;
        }
    } else {
        kfree(pi);
        kfree(io_buffer);
        return -1;
    }
}
