#ifndef __MYFS_DIRECTORY_H__
#define __MYFS_DIRECTORY_H__

#include <fs/myfs/inode.h>
#include <common/types.h>
#include <fs/myfs/fs.h>
#include <fs/myfs/fs_types.h>

#define MAX_FILE_NAME_LENGTH 32

/**
 * dir_t is an in memory structure
 */
typedef struct Directory {
    // pointer to my inode
    im_inode_t *im_inode;
    uint32_t dir_pos;
    uint8_t dir_buf[512];
} dir_t;

typedef struct DirectoryEntry {
    // + 1 is for '\0'
    char filename[MAX_FILE_NAME_LENGTH + 1];
    // inode number
    uint32_t i_no;
    // file type of this entry
    file_type_e f_type;
} dir_entry_t;

#define NR_DIR_ENTRY_PER_BLOCK (BLOCK_SIZE / sizeof(dir_entry_t))

#endif
