#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <mm/vmm.h>
#include <common/debug.h>
#include <common/types.h>

static void thread_run_func(thread_func_t func, void *args) {
    func(args);
}

static task_t *current_task = NULL;
static uint16_t num_tasks = 0;
// list of ready tasks
static list_t task_ready_list;
// list of tasks to exit
static list_t task_exit_list;

// list of all tasks
static list_t task_all_list;

// static void task_init(
//     task_t *task, const char *name, uint16_t prio,
//     thread_func_t func, void *args
// ) {
//     task->priority = prio;
//     task->ticks = prio;
//     strcpy(name, task->task_name);
//     task->stack_guard = 0x19971125;
//     task->kernel_stack = (uintptr_t)task + PAGE_SIZE;
//     task->kernel_stack -= sizeof(istk_t);
//     task->kernel_stack -= sizeof(thread_stk_t);
//     thread_stk_t *ts = (thread_stk_t *)(task->kernel_stack);
//     ts->eip = (uint32_t)thread_run_func;
//     ts->args = args;
//     ts->func = func;
//     ts->ret_addr_dummy = 0;
//     ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
//     __asm_volatile (
//         "mov esp, %0\n\t"
//         "xchg bx, bx\n\t"
//         "pop ebp\n\t"
//         "pop ebx\n\t"
//         "pop edi\n\t"
//         "pop esi\n\t"
//         "ret"
//         :
//         : "g"(task->kernel_stack)
//         :
//     );
// }

static task_t *task_create(
    const char *name, uint16_t prio, thread_func_t func,
    void *args
) {
    if (num_tasks >= MAX_TASKS) {
        return NULL;
    }
    task_t *task = (task_t *)k_get_free_page(GFP_ZERO);
    if (task == NULL) {
        return NULL;
    }
    ASSERT(strlen(name) < TASK_NAME_LEN);
    strcpy(task->task_name, name);
    task->status = TASK_READY;
    task->kernel_stack = (uintptr_t)task + PAGE_SIZE;
    task->kernel_stack -= sizeof(istk_t);
    task->kernel_stack -= sizeof(thread_stk_t);
    thread_stk_t *ts = (thread_stk_t *)(task->kernel_stack);
    ts->eip = (uint32_t)thread_run_func;
    ts->args = args;
    ts->func = func;
    ts->ret_addr_dummy = 0;
    ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
    task->priority = prio;
    task->ticks = prio;
    task->elapsed_ticks = 0;
    task->pg_dir = NULL;
    task->stack_guard = 0x19971125;
    return task;
}

int thread_start(
    const char *name, uint16_t prio,
    thread_func_t func, void *args
) {
    // task_t *task = (task_t *)k_get_free_page(GFP_ZERO);
    // task_init(task, name, 31, func, args);
    task_t *task = task_create(name, prio, func, args);
    if (task == NULL) {
        return -1;
    }
    ASSERT(!list_find(&task_ready_list, &(task->general_tag)));
    list_push_back(&task_ready_list, &(task->general_tag));
    ASSERT(!list_find(&task_all_list, &(task->list_all_tag)));
    list_push_back(&task_all_list, &(task->list_all_tag));
    return 0;
}

void thread_kmain(thread_func_t func, void *args) {
    static bool_t started = False;
    ASSERT(!started);
    started = True;
    task_t *task = task_create("kmain", 31, func, args);
    ASSERT(task != NULL);
    task->status = TASK_RUNNING;
    ASSERT(!list_find(&task_all_list, &(task->list_all_tag)));
    list_push_back(&task_all_list, &(task->list_all_tag));
    __asm_volatile (
        "mov esp, %0\n\t"
        "pop ebp\n\t"
        "pop ebx\n\t"
        "pop edi\n\t"
        "pop esi\n\t"
        "ret"
        :
        : "g"(task->kernel_stack)
        :
    );
}

void thread_init() {
    list_init(&task_ready_list);
    list_init(&task_all_list);
    list_init(&task_exit_list);
}

