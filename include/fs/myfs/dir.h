#ifndef __MYFS_DIRECTORY_H__
#define __MYFS_DIRECTORY_H__

#include <fs/myfs/inode.h>
#include <common/types.h>
#include <fs/myfs/fs_types.h>


/**
 * the dentries on disk should always be continuous
 */
typedef struct DirectoryEntry {
    // + 1 is for '\0'
    char filename[MAX_FILE_NAME_LENGTH + 1];
    // inode number
    uint32_t i_no;
    // file type of this entry
    file_type_e f_type;
} __attr_packed dir_entry_t ;

/**
 * @brief get dir entry by name
 * @return FSERR_NOERR if found; -FSERR_NONEXIST otherwise
 */
int get_dir_entry_by_name(
    const partition_t *part, const im_inode_t *dir, const char *name,
    dir_entry_t *dir_entry, void *io_buffer
);

/**
 * @brief write dir entry to disk
 */
int write_dir_entry(
    partition_t *part, im_inode_t *dir, dir_entry_t *dir_entry, void *buf
);

/**
 * @brief open a directory
 * 
 * @param part partition
 * @param i_no inode number of the dir
 * @return im_inode_t* pointer to the inode of the dir
 */
im_inode_t *dir_open(partition_t *part, uint32_t i_no);

/**
 * @brief close a directory
 * 
 * @param dir pointer to the inode of the dir
 */
void dir_close(im_inode_t *dir);

/**
 * @brief init dir entry
 */
void create_dir_entry(
    const char *filename, uint32_t i_no, file_type_e ft, dir_entry_t *de
);

/**
 * @brief create a dir in part under pdir with dirname
 */
int dir_create(
    partition_t *part, im_inode_t *pdir, const char *dirname, void *io_buf
);

/**
 * @brief open the root directory; only used when mount the partition
 */
void open_root_dir(partition_t *part);


void print_dentry(const dir_entry_t *dent);

#endif
