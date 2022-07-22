#ifndef __TASK_INFO_H__
#define __TASK_INFO_H__

#include <common/types.h>

typedef enum TaskStatus {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_SUSPENDING,
    TASK_STOPPED,
    TASK_FINISHED,
    TASK_DEAD,
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
