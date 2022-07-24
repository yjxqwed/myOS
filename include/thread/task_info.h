#ifndef __TASK_INFO_H__
#define __TASK_INFO_H__

#include <common/types.h>

typedef enum TaskStatus {
    // task is running on the cpu
    TASK_RUNNING,
    // task is ready to run on the cpu
    TASK_READY,
    // task is blocked by some external task (e.g. a disk io)
    TASK_BLOCKED,
    // task is waiting for its child process(es)
    TASK_WAITING,
    TASK_FINISHED,
    // task has called exited
    TASK_HANGING,
    // task's parent process doesn't wait for it
    TASK_ZOMBIED
} task_status_e;


#define TASK_NAME_LEN 16

typedef struct {
    pid_t task_id;
    pid_t parent_id;

    task_status_e status;
    uint32_t elapsed_ticks;

    int tty_no;

    char task_name[TASK_NAME_LEN];
} task_info_t;

#endif
