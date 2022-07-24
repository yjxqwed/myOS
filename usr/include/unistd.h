#ifndef __USR_UNISTD_H__
#define __USR_UNISTD_H__

/**
 *  myOS syscalls
 */

#include <common/types.h>
#include <thread/task_info.h>
#include <fs/fs.h>
#include <mm/mm_info.h>

#define	STDIN_FILENO    0  /* Standard input.  */
#define	STDOUT_FILENO   1  /* Standard output.  */
#define	STDERR_FILENO   2  /* Standard error output.  */

/**
 * @brief open a file
 * 
 * @return fd or -1
 */
int open(const char *filename, uint32_t flags);

/**
 * @brief close a fd
 * 
 * @return 0 or -1
 */
int close(int fd);

/**
 * @brief get info of a file
 * 
 * @param filename 
 * @param s user allocated buffer
 * @return int 0 or -1
 */
int stat(const char *filename, stat_t *s);

/**
 * @brief delete a file
 * 
 * @param filename 
 * @return int 0 or -1
 */
int unlink(const char *filename);

/**
 * @brief get info of all files
 * 
 * @param s user allocated buffer (should be large enough)
 * @return int number of objects or -1
 */
int list_files(stat_t *s);

/**
 * @brief change file pos
 * 
 * @param fd 
 * @param offset 
 * @param whence 
 * @return off_t new pos or -1
 */
off_t lseek(int fd, off_t offset, int whence);

/**
 * @brief write to fd
 */
int write(int fd, const void *buffer, size_t count);

/**
 * @brief read from fd
 */
int read(int fd, void *buffer, size_t count);

/**
 * @brief set program brk to addr
 * @return 0 on success; -1 on failure
 */
int brk(void *addr);

/**
 * @brief change program brk by increment byte(s)
 * @return new program brk
 */
void *sbrk(intptr_t increment);

/**
 * @brief sleep for ms milisec(s)
 */
void sleep(uint32_t ms);

/**
 * @brief get pid
 */
pid_t getpid();

/**
 * @brief get parent pid
 */
pid_t getppid();

/**
 * @brief create a children process
 */
pid_t create_process(const char *filename, char * const argv[]);

/**
 * @brief clear current screen
 */
void clear();

/**
 * @brief get information of all tasks
 * 
 * @param tis buffer
 * @param count size of the buffer
 * @return int number of ps
 */
int ps(task_info_t *tis, size_t count);

/**
 * @brief exit with status
 */
void _exit(int status);

/**
 * @brief wait for child processes
 */
pid_t wait(int *status);

#endif
