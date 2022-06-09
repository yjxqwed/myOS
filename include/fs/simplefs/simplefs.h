/**
 * @file simplefs.h
 * A very simple fs without directories
 */

#include <common/types.h>
#include <common/utils.h>

#define SECTOR_SIZE 512
#define SECTOR_SIZE_IN_BIT (8 * SECTOR_SIZE)
#define SIMPLE_FS_MAGIC 0x12345678
#define MAX_FILENAME_LENGTH 23

typedef struct FileDescriptor {
    // index is a sector of lbas; each lba is a data-sector
    uint32_t index_sector_lba;
    // number of bytes in this file
    uint32_t size;
    // filename
    char filename[MAX_FILENAME_LENGTH + 1];
} __attr_packed file_descriptor_t;

typedef struct SuperBlock {
    uint32_t fs_type;
    uint32_t part_start_lba;
    uint32_t sec_cnt;

    uint32_t sector_btmp_start_lba;
    uint32_t sector_btmp_sec_cnt;

} __attr_packed super_block_t;