#ifndef __SIMPLEFS_FILESYSTEM_H__
#define __SIMPLEFS_FILESYSTEM_H__

#include <common/types.h>
#include <common/utils.h>
#include <fs/fs.h>

void simplefs_init();
// don't use this
void simplefs_close();

int simplefs_file_open(const char *filename, uint32_t flags);
int simplefs_file_close(int fd);
int simplefs_file_read(int fd, void *buffer, size_t count);
int simplefs_file_write(int fd, const void *buffer, size_t count);
off_t simplefs_file_lseek(int fd, off_t offset, int whence);
int simplefs_file_delete(const char *filename);

int simplefs_file_stat(const char *filename, stat_t *s);
int simplefs_list_files(stat_t *s);

// void simplefs_init();

/**
 * @brief open a file
 */
int sys_open(const char *pathname, uint32_t flags);

/**
 * @brief close a file
 */
int sys_close(int fd);

/**
 * @brief read from fd
 * @return number of bytes read
 */
int sys_read(int fd, void *buffer, size_t count);

/**
 * @brief write to fd
 * @return number of bytes written
 */
int sys_write(int fd, const void *buffer, size_t count);

typedef int off_t;

/**
 * @brief set file position pointer
 * @return file position pointer or -1
 */
off_t sys_lseek(int fd, off_t offset, int whence);

/**
 * @brief delete a file
 * 
 * @param pathname 
 * @return int 0 for success, -1 for failure
 */
int sys_unlink(const char* pathname);

/**
 * @brief get info of a file
 * 
 * @param filename 
 * @param s stat_t object to store the result
 * @return int 0 for success, -1 for failure
 */
int sys_stat(const char *filename, stat_t *s);

/**
 * @brief list files
 * 
 * @param s a buffer to store stat_t objects for all files
 * @return int number of objects returned, or -1
 */
int sys_list_files(stat_t *s);


#endif
