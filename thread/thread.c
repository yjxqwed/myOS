#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>


void thread_run_func(thread_func_t func, void *args) {
    func(args);
}

static void task_init(task_t *task, const char *name, uint16_t prio) {
    memset(task, 0, sizeof(task_t));
    task->priority = prio;
    task->ticks = prio;
    strcpy(name, task->task_name);
    task->stack_guard = 0x19971125;
    task->kernel_stack = (uintptr_t) + PAGE_SIZE - 1;
}
