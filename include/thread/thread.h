#ifndef __THREAD_H__
#define __THREAD_H__

#include <common/types.h>

// thread_func_t is a function pointer to a function
// accepts void * and return void *
typedef void * (* thread_func_t)(void *);

typedef enum TaskStatus {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_SUSPENDING,
    TASK_DEAD,
} TaskStatus;

typedef struct InterruptStack {
    
} int_stk_t;

#endif