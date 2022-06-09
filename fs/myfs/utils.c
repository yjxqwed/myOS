#include <fs/myfs/fs_types.h>
#include <fs/myfs/utils.h>
#include <common/types.h>


int analyze_path(const char *pathname, path_info_t *pi) {
    int path_len = strlen(pathname);
    if (path_len > MAX_PATH_LENGTH) {
        return -FSERR_PATHTOOLONG;
    }
    // init path info
    pi->abs = False;
    pi->depth = 0;
    for (int i = 0; i < MAX_PATH_DEPTH; i++) {
        pi->path[i][0] = '\0';
    }

    int i = 0;
    char filename[MAX_FILE_NAME_LENGTH + 1] = "";

    if (pathname[0] == '/') {
        pi->abs = True;
        while (1) {
            while (pathname[i] == '/') {
                i++;
            }
            int j;
            for (
                j = 0;
                pathname[i] != '/' &&
                pathname[i] != '\0' &&
                j <= MAX_FILE_NAME_LENGTH;
                i++, j++
            ) {
                filename[j] = pathname[i];
            }
            if (j > MAX_FILE_NAME_LENGTH) {
                return -FSERR_FILENAMETOOLONG;
            }
            filename[j] = '\0';
            if (!strcmp(filename, ".") || !strcmp(filename, "..")) {
                continue;
            } else {
                while (pathname[i] == '/') {
                    i++;
                }
                break;
            }
        }
    }

    if (filename[0] != '\0') {
        strcpy(filename, pi->path[pi->depth]);
        (pi->depth)++;
        filename[0] = '\0';
    }

    while (pathname[i] != '\0') {
        int j;
        for (
            j = 0;
            pathname[i] != '/' &&
            pathname[i] != '\0' &&
            j <= MAX_FILE_NAME_LENGTH;
            i++, j++
        ) {
            filename[j] = pathname[i];
        }
        while (pathname[i] == '/') {
            i++;
        }
        if (j > MAX_FILE_NAME_LENGTH) {
            return -FSERR_FILENAMETOOLONG;
        }
        filename[j] = '\0';
        if (!strcmp(filename, ".")) {
            continue;
        }
        strcpy(filename, pi->path[pi->depth]);
        (pi->depth)++;
        if (pi->depth > MAX_PATH_DEPTH) {
            return -FSERR_PATHTOODEEP;
        }
    }

    if (pathname[path_len - 1] == '/') {
        pi->isdir = True;
    } else if (strcmp(pi->path[pi->depth - 1], "..") == 0) {
        pi->isdir = True;
    } else if (pi->depth == 0) {
        pi->isdir = True;
    } else {
        pi->isdir = False;
    }

    return FSERR_NOERR;
}

// void print_path_info(const path_info_t *pi) {
//     printf("  abs: %s\n", pi->abs ? "true" : "false");
//     printf("  depth: %d\n", pi->depth);
//     for (int i = 0; i < pi->depth; i++) {
//         printf("  [%d]: %s\n", i, pi->path[i]);
//     }
// }


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

void block_reclaim(partition_t *part, int blk_no) {
    bitmap_set(&(part->block_btmp), blk_no, 0);
}

void inode_btmp_sync(partition_t *part, int inode_bit_idx) {
    // uint32_t sec_off = inode_bit_idx / SECTOR_SIZE_IN_BIT;
    // uint32_t byte_off = sec_off * SECTOR_SIZE;

    // uint32_t lba = part->sb->inode_btmp_start_lba + sec_off;
    // void *data = part->inode_btmp.bits_ + byte_off;
    // // dirty_blocks_add(part, lba, data);
    // ata_write(part->my_disk, lba, data, 1);
}

void block_btmp_sync(partition_t *part, int blk_bit_idx) {
    // uint32_t sec_off = blk_bit_idx / SECTOR_SIZE_IN_BIT;
    // uint32_t byte_off = sec_off * SECTOR_SIZE;

    // uint32_t lba = part->sb->block_btmp_start_lba + sec_off;
    // void *data = part->block_btmp.bits_ + byte_off;
    // // dirty_blocks_add(part, lba, data);
    // ata_write(part->my_disk, lba, data, 1);
}

void partition_block_read(partition_t *part, uint32_t blk_id, void *buf, uint32_t blk_cnt) {
    uint32_t lba = part->start_lba + blk_id * NR_SECTORS_PER_BLOCK;
    uint32_t sec_cnt = blk_cnt * NR_SECTORS_PER_BLOCK;
    ASSERT(lba + sec_cnt <= part->sec_cnt);
    ata_read(part->my_disk, lba, buf, sec_cnt);
}

void partition_block_write(partition_t *part, uint32_t blk_id, void *buf, uint32_t blk_cnt) {
    uint32_t lba = part->start_lba + blk_id * NR_SECTORS_PER_BLOCK;
    uint32_t sec_cnt = blk_cnt * NR_SECTORS_PER_BLOCK;
    ASSERT(lba + sec_cnt <= part->sec_cnt);
    ata_write(part->my_disk, lba, buf, sec_cnt);
}