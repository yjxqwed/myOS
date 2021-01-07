#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <thread/thread.h>

/**
 * @brief start a new process
 * @param filename file to the binary
 * @param name name of the process
 * @param tty_no tty of this process
 */
task_t *process_execute(char *filename, char *name, int tty_no);


/**
 * @brief install the global fd into task's own fd_table
 * @return private fd
 */
int install_global_fd(int gfd);

#endif
