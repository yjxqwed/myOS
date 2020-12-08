#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <mm/kvmm.h>
#include <common/debug.h>
#include <common/types.h>
#include <kprintf.h>

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

static __PAGE_ALLIGNED uint8_t reserved_pcb[PAGE_SIZE];
uint32_t kmain_stack_top = (uintptr_t)reserved_pcb + PAGE_SIZE;
static task_t *kmain = NULL;

static void schedule();

static task_t *current_task = NULL;
static uint16_t num_tasks = 0;
// list of ready tasks
static list_t task_ready_list;
// list of tasks to exit
static list_t task_exit_list;

// list of all tasks
static list_t task_all_list;


static void thread_run_func(thread_func_t func, void *args) {
    enable_int();
    func(args);
    // when the function is done executed
    disable_int();
    current_task->status = TASK_DEAD;

    list_node_t *p = list_pop_front(&(current_task->join_list));
    while (p) {
        ASSERT(!list_find(&task_ready_list, p));
        list_push_back(&task_ready_list, p);
        task_t *t = __list_node_struct(task_t, general_tag, p);
        t->status = TASK_READY;
        p = list_pop_front(&(current_task->join_list));
    }

    schedule();
}

static void task_init(
    task_t *task, const char *name, uint16_t prio
) {
    task->priority = prio;
    task->ticks = prio;
    strcpy(name, task->task_name);
    task->stack_guard = 0x19971125;
    task->kernel_stack = (uintptr_t)task + PAGE_SIZE;
    task->status = TASK_READY;
    list_init(&(task->join_list));
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
    if (strlen(name) >= TASK_NAME_LEN) {
        return NULL;
    }
    task_init(task, name, prio);
    // task->kernel_stack -= sizeof(istk_t);

    // init thread stack
    task->kernel_stack -= sizeof(thread_stk_t);
    thread_stk_t *ts = (thread_stk_t *)(task->kernel_stack);
    ts->eip = (uint32_t)thread_run_func;
    ts->ret_addr_dummy = 0;
    ts->func = func;
    ts->args = args;

    ts->ebp = ts->ebx = ts->esi = ts->edi = 0;
    // end of init thread stack

    task->elapsed_ticks = 0;
    task->pg_dir = NULL;
    return task;
}

task_t *thread_start(
    const char *name, uint16_t prio,
    thread_func_t func, void *args
) {
    task_t *task = task_create(name, prio, func, args);
    if (task == NULL) {
        return NULL;
    }
    ASSERT(!list_find(&task_ready_list, &(task->general_tag)));
    list_push_back(&task_ready_list, &(task->general_tag));
    ASSERT(!list_find(&task_all_list, &(task->list_all_tag)));
    list_push_back(&task_all_list, &(task->list_all_tag));
    return task;
}

void thread_kmain() {
    kmain = (task_t *)reserved_pcb;
    task_init(kmain, "kmain", 31);
    ASSERT(!list_find(&task_all_list, &(kmain->list_all_tag)));
    list_push_back(&task_all_list, &(kmain->list_all_tag));
    kmain->status = TASK_RUNNING;
    current_task = kmain;
}



void thread_init() {
    list_init(&task_ready_list);
    list_init(&task_all_list);
    list_init(&task_exit_list);
}

static void clear_exit_q() {
    list_node_t *p = list_pop_front(&task_exit_list);
    while (p) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        list_erase(&(t->list_all_tag));
        k_free_page(t);
        p = list_pop_front(&task_exit_list);
    }
}

static void schedule() {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    clear_exit_q();
    task_t *old = current_task;
    if (old->status == TASK_RUNNING) {
        ASSERT(!list_find(&task_ready_list, &(old->general_tag)));
        list_push_back(&task_ready_list, &(old->general_tag));
        old->status = TASK_READY;
        old->ticks = old->priority;
    } else if (old->status == TASK_DEAD) {
        ASSERT(!list_find(&task_exit_list, &(old->general_tag)));
        list_push_back(&task_exit_list, &(old->general_tag));
    } else if (old->status == TASK_WAITING) {

    }
    ASSERT(!list_empty(&task_ready_list));
    task_t *next = NULL;
    next = __list_node_struct(
        task_t, general_tag, list_pop_front(&task_ready_list)
    );
    ASSERT(next->stack_guard == 0x19971125);
    ASSERT(next->status == TASK_READY);
    next->status = TASK_RUNNING;
    current_task = next;
    extern void switch_to(task_t *prev, task_t *next);
    switch_to(old, next);
}

void time_scheduler() {
    ASSERT(current_task != NULL && current_task->stack_guard == 0x19971125);
    (current_task->elapsed_ticks)++;
    if (current_task->ticks == 0) {
        schedule();
    } else {
        (current_task->ticks)--;
    }
}

void print_all_tasks() {
    kprintf(KPL_DEBUG, "all: head->");
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

void print_ready_tasks() {
    kprintf(KPL_DEBUG, "ready: head->");
    for (
        list_node_t *p = task_ready_list.head.next;
        p != &(task_ready_list.tail);
        p = p->next
    ) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        kprintf(KPL_DEBUG, "%s->", t->task_name);
    }
    kprintf(KPL_DEBUG, "tail");
}

void print_exit_tasks() {
    kprintf(KPL_DEBUG, "exit: head->");
    for (
        list_node_t *p = task_exit_list.head.next;
        p != &(task_exit_list.tail);
        p = p->next
    ) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        kprintf(KPL_DEBUG, "%s->", t->task_name);
    }
    kprintf(KPL_DEBUG, "tail");
}

void thread_join(task_t *task) {
    INT_STATUS old_status = disable_int();
    if (task == NULL || task->status == TASK_DEAD) {
        enable_int();
        return;
    }
    ASSERT(task != current_task);
    ASSERT(!list_find(&(task->join_list), &(current_task->general_tag)));
    list_push_back(&(task->join_list), &(current_task->general_tag));
    current_task->status = TASK_WAITING;
    schedule();
    set_int_status(old_status);
}

void thread_block_self(task_status_e status) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(
        status == TASK_BLOCKED ||
        status == TASK_SUSPENDING ||
        status == TASK_WAITING
    );
    current_task->status = status;
    schedule();
}

void thread_unblock(task_t *task) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(
        task->status == TASK_BLOCKED ||
        task->status == TASK_SUSPENDING ||
        task->status == TASK_WAITING
    );
    ASSERT(!list_find(&task_ready_list, &(task->general_tag)));
    list_push_front(&task_ready_list, &(task->general_tag));
    task->status = TASK_READY;
}

task_t *get_current_thread() {
    return current_task;
}