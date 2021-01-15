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

// max path depth is 16
#define MAX_PATH_DEPTH 16

// max path length is 512
#define MAX_PATH_LENGTH 512

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

// open file flags
typedef enum OFlags {
    // read only
    O_RDONLY = 0,
    // write only
    O_WRONLY = 1,
    // read & write
    O_RDWR   = 2,
    // create
    O_CREAT  = 4
} oflags_e;

enum {
    FSERR_NOERR = 0,
    // no such partition
    FSERR_NOPART = 1,
    FSERR_NOMEM = 2,
    // file not exist
    FSERR_NONEXIST,
    // file not a dir
    FSERR_NOTDIR,
    // no inode in a partition
    FSERR_NOINODE,
    // no block in a partition
    FSERR_NOBLOCK,
    // no global fd (system level)
    FSERR_NOGLOFD,
    // no local fd (per process)
    FSERR_NOLOCFD,
    // directory is full
    FSERR_DIRFULL,
};

#endif
