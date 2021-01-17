#ifndef __MYFS_DIRECTORY_H__
#define __MYFS_DIRECTORY_H__

#include <fs/myfs/inode.h>
#include <common/types.h>
#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>


typedef struct DirectoryEntry {
    // + 1 is for '\0'
    char filename[MAX_FILE_NAME_LENGTH + 1];
    // inode number
    uint32_t i_no;
    // file type of this entry
    file_type_e f_type;
} dir_entry_t;

/**
 * @brief get dir entry by name
 */
int get_dir_entry_by_name(
    const partition_t *part, const dir_t *dir, const char *name,
    dir_entry_t *dir_entry, void *io_buffer
);

/**
 * @brief write dir entry to disk
 */
int write_dir_entry(
    partition_t *part, dir_t *dir, dir_entry_t *dir_entry, void *buf
);

/**
 * @brief open a directory
 */
dir_t *dir_open(partition_t *part, uint32_t i_no);

/**
 * @brief close a directory
 */
void dir_close(dir_t *dir);

/**
 * @brief init dir entry
 */
void create_dir_entry(
    const char *filename, uint32_t i_no,
    file_type_e ft, dir_entry_t *de
);

void open_root_dir(partition_t *part);

#define NR_DIR_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(dir_entry_t))

#endif
