#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <thread/thread.h>

// a process can open at most 8 files at the same time
#define NR_OPEN 8

/**
 * @brief start a new process
 * @param filename file to the binary
 * @param name name of the process
 * @param tty_no tty of this process
 */
task_t *process_execute(const char *filename, const char *name, int tty_no);

pid_t sys_getpid();
pid_t sys_getppid();

#endif
