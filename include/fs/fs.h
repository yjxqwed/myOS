#ifndef __FS_H__
#define __FS_H__

#include <common/types.h>

// open file flags
enum {
    // read only
    O_RDONLY = 0,
    // write only
    O_WRONLY = 1,
    // read & write
    O_RDWR   = 2,
    // create
    O_CREAT  = 4,
};

// for lseek
enum {
    SEEK_SET = 0,
    SEEK_CUR = 1,
    SEEK_END = 2
};

// Reserved FDs
enum {
    FD_STDIN  = 0,
    FD_STDOUT = 1,
    FD_STDERR = 2
};

#define MAX_FILENAME_LENGTH 23

typedef struct {
    char filename[MAX_FILENAME_LENGTH + 1];
    uint32_t file_id;
    uint32_t size;
    uint32_t blocks;
} stat_t;

#endif
