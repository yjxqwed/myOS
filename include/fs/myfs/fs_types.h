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
 * inode is for information node or index node
 * it is the metadata of any file(directory) on disk
 * sizeof(inode_t) should be a divisor of 512
 */

#define NR_BLOCKS_PER_INODE 14

typedef struct INode {
    // inode number
    uint32_t i_no;

    /**
     * If this inode is for a file, i_size is the size of the file
     * If this inode is for a dir, i_size if the sum of all dir entries' size
     */
    uint32_t i_size;

    /**
     * i_blocks is the array of lbas of data blocks
     * i_blocks[0] ~ i_blocks[11] are direct blocks
     * i_blocks[12] is the one-level indirect block
     * i_blocks[13] is the two-level indirect block
     */
    uint32_t i_blocks[NR_BLOCKS_PER_INODE];
} __attr_packed inode_t;


// the in memory wrapper of inode
typedef struct InMemoryINode {
    // on disk inode
    inode_t inode;
    // number of times of this file being opened
    uint32_t i_open_times;
    // one file can be written by a single writer.
    bool_t write_deny;
    // for linked list to use
    list_node_t i_tag;
    // if true, need to sync
    bool_t dirty;
} im_inode_t;

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
