#ifndef __THREAD_H__
#define __THREAD_H__

#include <common/types.h>
#include <list.h>
#include <arch/x86.h>

#define MAX_TASKS 256

typedef uint16_t thread_id_t;

// thread_func_t is a function pointer to a function
// accepts void * and return void *
typedef void (* thread_func_t)(void *);

typedef enum TaskStatus {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_SUSPENDING,
    TASK_STOPPED,
    TASK_FINISHED,
    TASK_DEAD,
} task_status_e;


#define TASK_NAME_LEN 16

// the task struct
typedef struct Task {

    // the stack pointer to the stack used by this task
    // in the kernel mode
    uintptr_t kernel_stack;

    uint16_t task_id;
    uint16_t parent_id;
    char task_name[TASK_NAME_LEN];
    task_status_e status;

    uint16_t priority;
    uint16_t ticks;
    // number of ticks this task has been running
    uint32_t elapsed_ticks;

    // for general use
    list_node_t general_tag;
    // node in the list of all tasks
    list_node_t list_all_tag;

    // process has its own page directory
    // (virtual address)
    pde_t *pg_dir;

    // list of threads join this thread
    list_t join_list;

    // msec left to sleep
    uint32_t sleep_msec;

    // a magic number to guard this struct
    uint32_t stack_guard;

} task_t;


// start a new thread; return 0 on scuess
// @param name name of this thread
// @param prio priority of this thread
// @param func the function to be run
// @param args the parameter(s) of func
task_t *thread_start(
    const char *name, uint16_t prio,
    thread_func_t func, void *args
);

// wait for task
void thread_join(task_t *task);

// init thread related structures
void thread_init();

// init the main thread's task_struct
// main thread is the execution flow that have been executing
// after the bootstrap
void thread_kmain();

void time_scheduler();

void print_all_tasks();
void print_ready_tasks();
void print_exit_tasks();
void print_sleeping_tasks();

void thread_yield();

// thread block self and set self status 
// only called when int is disabled
// @param status the target status
void thread_block_self(task_status_e status);

// put task in the ready queue 
// only called when int is disabled 
// and when task is blocked
// @param task the task to be unblocked
void thread_unblock(task_t *task);

// get the running thread
task_t *get_current_thread();

// @brief sleep for milisec, if msec is 0, does nothing
// @param msec time in milisec
void thread_msleep(uint32_t msec);

void sleep_manage();

#endif
