#ifndef __THREAD_H__
#define __THREAD_H__

#include <common/types.h>
#include <list.h>

// thread_func_t is a function pointer to a function
// accepts void * and return void *
typedef void (* thread_func_t)(void *);

typedef enum TaskStatus {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_SUSPENDING,
    TASK_DEAD,
} task_status_e;

typedef struct ThreadStack {



} thread_stk_t;

#define TASK_NAME_LEN 16

typedef struct task_struct {

    uint16_t task_id;
    uint16_t parent_id;
    char task_name[TASK_NAME_LEN];
    task_status_e status;

    // the stack pointer to the stack used by this task
    // in the kernel mode
    uintptr_t kernel_stack;

    uint16_t priority;
    uint16_t ticks;

    // a magic number to guard this struct
    uint32_t stack_guard;

} task_t;

#endif
