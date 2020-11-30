#ifndef __THREAD_H__
#define __THREAD_H__

#include <common/types.h>
#include <list.h>
#include <arch/x86.h>

#define MAX_TASKS 256

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
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    uint32_t eip;

    uint32_t ret_addr_dummy;
    thread_func_t func;
    void *args;
} thread_stk_t;

#define TASK_NAME_LEN 16

typedef struct task_struct {

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

    // a magic number to guard this struct
    uint32_t stack_guard;

} task_t;


// start a new thread; return 0 on scuess
// @param name name of this thread
// @param prio priority of this thread
// @param func the function to be run
// @param args the parameter(s) of func
int thread_start(
    const char *name, uint16_t prio,
    thread_func_t func, void *args
);

// init thread related structures
void thread_init();

void thread_kmain();

void time_scheduler();

void print_all_tasks();

#endif
