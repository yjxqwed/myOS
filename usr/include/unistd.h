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
int write(int fd, void *buffer, size_t count);

/**
 * @brief set program brk to __addr
 * @return new brk on success; old brk on failure
 */
void *brk(uintptr_t __addr);

/**
 * @brief change program brk by __delta byte(s)
 * @return new program brk
 */
void *sbrk(intptr_t __delta);

/**
 * @brief sleep for ms milisec(s)
 */
void sleep(uint32_t ms);

#endif
