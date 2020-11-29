#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <mm/vmm.h>


static void thread_run_func(thread_func_t func, void *args) {
    func(args);
}

static void task_init(
    task_t *task, const char *name, uint16_t prio,
    thread_func_t func, void *args) {
    task->priority = prio;
    task->ticks = prio;
    strcpy(name, task->task_name);
    task->stack_guard = 0x19971125;
    task->kernel_stack = (uintptr_t)task + PAGE_SIZE;
    task->kernel_stack -= sizeof(istk_t);
    task->kernel_stack -= sizeof(thread_stk_t);
    thread_stk_t *ts = (thread_stk_t *)(task->kernel_stack);
    ts->eip = (uint32_t)thread_run_func;
    ts->args = args;
    ts->func = func;
    ts->ret_addr_dummy = 0;
    ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
    __asm_volatile (
        "mov esp, %0\n\t"
        "xchg bx, bx\n\t"
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

void thread_start(const char *name, thread_func_t func, void *args) {
    task_t *task = (task_t *)k_get_free_page(GFP_ZERO);
    task_init(task, name, 31, func, args);
}
