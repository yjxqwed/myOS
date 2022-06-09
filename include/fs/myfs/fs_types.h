#ifndef __MYFS_FS_TYPES_H__
#define __MYFS_FS_TYPES_H__

/**
 * @file fs/myfs/fs_types.h
 * @brief some definitions of myfs
 */

#include <common/types.h>
#include <list.h>
#include <common/utils.h>

#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT (8 * SECTOR_SIZE)

// myfs use a sector as a block
#define NR_SECTORS_PER_BLOCK 1
#define BLOCK_SIZE (NR_SECTORS_PER_BLOCK * SECTOR_SIZE)
#define BLOCK_SIZE_IN_BIT (8 * BLOCK_SIZE)

// each partition can have at most 4096 files
#define MAX_FILE_CNT_PER_PART 4096

// max path depth is 10
#define MAX_PATH_DEPTH 10

// max path length is 512
#define MAX_PATH_LENGTH 512

#define MAX_FILE_NAME_LENGTH 55

#define BOOT_BLOCK_BLK_CNT 1
#define SUPER_BLOCK_START_BLK_ID BOOT_BLOCK_BLK_CNT
#define SUPER_BLOCK_BLK_CNT 1

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
    O_CREAT  = 4,
    // open a dir only
    O_DIRECTORY = 8
} oflags_e;


typedef enum SeekWhence {
    SEEK_SET = 0,
    SEEK_CUR = 1,
    SEEK_END = 2
} seek_wence_e;


enum {
    FSERR_NOERR = 0,
    // no such partition
    FSERR_NOPART = 2,
    // mem shortage
    FSERR_NOMEM,
    // file already exists
    FSERR_EXIST,
    // file doesn't exist
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
    // path too long
    FSERR_PATHTOOLONG,
    // path too deep
    FSERR_PATHTOODEEP,
    // file name too long
    FSERR_FILENAMETOOLONG,
    // if sys_open a dir, return this
    FSERR_DIRECTORY,
    // exclusive write (a file can only be
    //   written by at most one process at any moment)
    FSERR_EXCWRITE,

    // process provides a bad fd
    FSERR_BADFD,

    // lseek bad offset
    FSERR_BADOFF,
    // lseek bad whence
    FSERR_BADWHENCE,

    // out of range
    FSERR_RANGE
};


/**
 * dir_t is an in memory structure
 */
// typedef struct Directory {
//     // pointer to my inode
//     im_inode_t *im_inode;
//     uint32_t dir_pos;
//     uint8_t dir_buf[512];
// } dir_t;

typedef int off_t;

#endif
