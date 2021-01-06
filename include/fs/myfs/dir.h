#ifndef __MYFS_DIRECTORY_H__
#define __MYFS_DIRECTORY_H__

#include <fs/myfs/inode.h>
#include <common/types.h>
#include <fs/myfs/fs.h>

#define MAX_FILE_NAME_LENGTH 32

/**
 * dir_t is an in memory structure
 */
typedef struct Directory {
    // pointer to my inode
    inode_t *inode;
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

#endif
