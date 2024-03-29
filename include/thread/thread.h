#ifndef __THREAD_H__
#define __THREAD_H__

typedef struct Task task_t;

#include <common/types.h>
#include <lib/list.h>
#include <arch/x86.h>
#include <mm/vmm.h>

#include <thread/task_info.h>
// #include <fs/myfs/fs_types.h>

#define MAX_TASKS 256

// thread_func_t is a function pointer to a function
// that accepts void * and returns void *
typedef void *(* thread_func_t)(void *);

// the task struct
struct Task {

    // the stack pointer to the stack used by this task
    // in the kernel mode
    uintptr_t kernel_stack;

    pid_t task_id;
    pid_t parent_id;

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

    // process has its own address space
    vmm_t *vmm;

    // list of threads that join this thread
    list_t join_list;

    // msec left to sleep
    uint32_t sleep_msec;

    // the associated tty; -1 = no tty
    int tty_no;

    // file discriptor table
    int *fd_table;

    // current work dir inode number
    int cwd_inode_no;

    // true if this is a user process
    bool_t is_user_process;

    // exit status used by parent
    uint8_t exit_status;

    // a magic number to guard this struct
    uint32_t stack_guard;
};


// start a new thread
// @param name name of this thread
// @param prio priority of this thread
// @param func the function to be run
// @param args the parameter(s) of func
task_t *thread_start(
    const char *name, uint16_t prio, thread_func_t func, void *args
);

// wait for task
void thread_join(task_t *task);

// init thread related structures
void thread_init();

void time_scheduler();

void print_all_tasks();
void print_ready_tasks();
void print_exit_tasks();
void print_sleeping_tasks();
void print_task_nums();


inline void task_push_back_ready(task_t *t);
inline void task_push_back_all(task_t *t);

task_t *task_create(
    const char *name, uint16_t prio, thread_func_t func, void *args
);

void task_destroy(task_t *task);

// associate task with tty
void task_assign_tty(task_t *task, int tty_no);

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

// @brief sleep for milisec, if msec is 0, yield
// @param msec time in milisec
void thread_msleep(uint32_t msec);

void sleep_manage();

/**
 * @brief allocate a pid; return -1 if no available pid
 */
pid_t pid_alloc();

/**
 * @brief free an allocated pid
 */
void pid_free(pid_t pid);

/**
 * @brief find task_t by pid
 */
task_t *pid2task(pid_t pid);


/**
 * @brief find a child process by pid
 */
task_t *find_child_process(pid_t pid);

/**
 * @brief get tasks' information
 */
int sys_ps(task_info_t *tis, size_t count);

#endif
