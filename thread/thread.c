#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <mm/vmm.h>
#include <common/debug.h>
#include <common/types.h>
#include <kprintf.h>

static __PAGE_ALLIGNED uint8_t reserved_pcb[PAGE_SIZE];
uint32_t kmain_stack_top = reserved_pcb + PAGE_SIZE;
static task_t *kmain = NULL;

static void thread_run_func(thread_func_t func, void *args) {
    enable_int();
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

static void task_init(
    task_t *task, const char *name, uint16_t prio
) {
    task->priority = prio;
    task->ticks = prio;
    strcpy(name, task->task_name);
    task->stack_guard = 0x19971125;
    task->kernel_stack = (uintptr_t)task + PAGE_SIZE;
    task->status = TASK_READY;
}

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

void thread_kmain() {
    kmain = (task_t *)reserved_pcb;
    task_init(kmain, "kmain", 31);
    ASSERT(!list_find(&task_all_list, &(kmain->list_all_tag)));
    list_push_back(&task_all_list, &(kmain->list_all_tag));
    kmain->status = TASK_RUNNING;
}



void thread_init() {
    list_init(&task_ready_list);
    list_init(&task_all_list);
    list_init(&task_exit_list);
}

static void switch_thread(task_t *prev, task_t *next) {
    next->status = TASK_RUNNING;
    current_task = next;
    MAGICBP;
    __asm_volatile (
        "mov %0, esp\n\t"
        "mov esp, %1"
        : "=g"(prev->kernel_stack)
        : "g"(next->kernel_stack)
        :
    );
}

static void schedule() {
    task_t *old = current_task;
    if (old->status == TASK_RUNNING) {
        ASSERT(!list_find(&task_ready_list, &(old->general_tag)));
        list_push_back(&task_ready_list, &(old->general_tag));
        old->status = TASK_READY;
        old->ticks = old->priority;
    }
    ASSERT(!list_empty(&task_ready_list));
    task_t *next = NULL;
    next = __list_node_struct(
        task_t, general_tag, list_pop_front(&task_ready_list)
    );
    ASSERT(next->status == TASK_READY);
    switch_thread(old, next);
}

void time_scheduler() {
    // make sure the interrupt is disabled
    ASSERT(get_int_status() == INTERRUPT_OFF);
    // make sure the thread's stack is correct
    ASSERT(current_task != NULL && current_task->stack_guard == 0x19971125);
    (current_task->elapsed_ticks)++;
    if (current_task->ticks == 0) {
        schedule();
    } else {
        (current_task->ticks)--;
    }
}

void print_all_tasks() {
    kprintf(KPL_DEBUG, "head->");
    for (
        list_node_t *p = task_all_list.head.next;
        p != &(task_all_list.tail);
        p = p->next
    ) {
        task_t *t = __list_node_struct(task_t, list_all_tag, p);
        kprintf(KPL_DEBUG, "%s->", t->task_name);
    }
    kprintf(KPL_DEBUG, "tail");
}
