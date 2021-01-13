#ifndef __MYFS_FS_TYPES_H__
#define __MYFS_FS_TYPES_H__

/**
 * @file fs/myfs/fs_types.h
 * @brief some definitions of myfs
 */

#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT 4096

// myfs use a sector as a block
#define BLOCK_SIZE (1 * SECTOR_SIZE)
#define BLOCK_SIZE_IN_BIT (1 * SECTOR_SIZE_IN_BIT)

// each partition can have at most 4096 files
#define MAX_FILE_CNT_PER_PART 4096

#define BOOT_BLOCK_SEC_CNT 1
#define SUPER_BLOCK_SEC_CNT 1

#define FS_MAGIC 0x19971125

typedef enum FileType {
    // unknown file type
    FT_NONE,
    // regular file
    FT_REGULAR,
    // directory
    FT_DIRECTORY
} file_type_e;

enum {
    FSERR_NOERR = 0,
    // no such partition
    FSERR_NOPART = 1
};

#endif
