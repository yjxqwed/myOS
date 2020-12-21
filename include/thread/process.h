#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <thread/thread.h>

// @brief start a new process
// @param filename file to the binary
// @name name of the process
task_t *process_execute(char *filename, char *name);

#endif