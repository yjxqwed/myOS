#ifndef __USR_UNISTD_H__
#define __USR_UNISTD_H__

/**
 *  myOS syscalls
 */

#include <common/types.h>


#define	STDIN_FILENO    0  /* Standard input.  */
#define	STDOUT_FILENO   1  /* Standard output.  */
#define	STDERR_FILENO   2  /* Standard error output.  */

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

#endif
