#ifndef __MYFS_FILESYSTEM_H__
#define __MYFS_FILESYSTEM_H__

#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT 4096
// each partition can have at most 4096 files
#define MAX_FILE_CNT_PER_PART 4096

typedef enum FileType {
    // unknown file type
    FT_NONE,
    // regular file
    FT_REGULAR,
    // directory
    FT_DIRECTORY
} file_type_e;

void fs_init();

#endif
