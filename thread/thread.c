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

// @brief schedule the tasks
static void schedule();

static task_t *current_task = NULL;
static uint16_t num_tasks = 0;

// list of ready tasks
static list_t task_ready_list;
// list of tasks to exit
static list_t task_exit_list;
// list of all tasks
static list_t task_all_list;
// list of sleeping tasks
static list_t sleeping_list;

static inline void task_push_back(list_t *l, task_t *t) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(l != &task_all_list);
    ASSERT(!list_find(l, &(t->general_tag)));
    list_push_back(l, &(t->general_tag));
}

static inline void task_push_front(list_t *l, task_t *t) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(l != &task_all_list);
    ASSERT(!list_find(l, &(t->general_tag)));
    list_push_front(l, &(t->general_tag));
}

static inline task_t *task_pop_front(list_t *l) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(l != &task_all_list);
    list_node_t *p = list_pop_front(l);
    if (p) {
        return __list_node_struct(task_t, general_tag, p);
    } else {
        return NULL;
    }
}

static inline task_t *task_pop_back(list_t *l) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    ASSERT(l != &task_all_list);
    list_node_t *p = list_pop_back(l);
    if (p) {
        return __list_node_struct(task_t, general_tag, p);
    } else {
        return NULL;
    }
}

static void thread_run_func(thread_func_t func, void *args) {
    enable_int();
    func(args);
    // when the function is done executed
    disable_int();
    current_task->status = TASK_STOPPED;

    list_node_t *p = list_pop_front(&(current_task->join_list));
    while (p) {
        // ASSERT(!list_find(&task_ready_list, p));
        // list_push_back(&task_ready_list, p);
        task_t *t = __list_node_struct(task_t, general_tag, p);
        task_push_back(&task_ready_list, t);
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
    task->sleep_msec = 0;
    list_init(&(task->join_list));
}

static task_t *task_create(
    const char *name, uint16_t prio, thread_func_t func,
    void *args
) {
    // if (num_tasks >= MAX_TASKS) {
    //     return NULL;
    // }
    task_t *task = (task_t *)k_get_free_pages(1, GFP_ZERO);
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
    INT_STATUS old_status = disable_int();
    ASSERT(!list_find(&task_ready_list, &(task->general_tag)));
    list_push_back(&task_ready_list, &(task->general_tag));
    ASSERT(!list_find(&task_all_list, &(task->list_all_tag)));
    list_push_back(&task_all_list, &(task->list_all_tag));
    set_int_status(old_status);
    return task;
}

void thread_kmain() {
    kmain = (task_t *)reserved_pcb;
    task_init(kmain, "kmain", 31);
    INT_STATUS old_status = disable_int();
    ASSERT(!list_find(&task_all_list, &(kmain->list_all_tag)));
    list_push_back(&task_all_list, &(kmain->list_all_tag));
    set_int_status(old_status);
    kmain->status = TASK_RUNNING;
    current_task = kmain;
}


void thread_init() {
    list_init(&task_ready_list);
    list_init(&task_all_list);
    list_init(&task_exit_list);
    list_init(&sleeping_list);
}

static void clear_exit_q() {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    list_node_t *p = list_pop_front(&task_exit_list);
    while (p) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        list_erase(&(t->list_all_tag));
        k_free_pages(t, 1);
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
    } else if (old->status == TASK_STOPPED) {
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
    if (old != next) {
        extern void switch_to(task_t *prev, task_t *next);
        switch_to(old, next);
    }
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

static void print_tasks(const list_t *l) {
    char *name;
    if (l == &task_all_list) {
        name = "all";
    } else if (l == &task_ready_list) {
        name = "ready";
    } else if (l == &task_exit_list) {
        name = "exit";
    } else if (l == &sleeping_list) {
        name = "sleeping";
    } else {
        kprintf(KPL_PANIC, "Unknown List 0x%X\n", l);
        return;
    }
    kprintf(KPL_DEBUG, "%s: head->", name);
    for (
        list_node_t *p = l->head.next;
        p != &(l->tail);
        p = p->next
    ) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        kprintf(KPL_DEBUG, "{0x%X, %s}->", t, t->task_name);
    }
    kprintf(KPL_DEBUG, "tail\n");
}

void print_all_tasks() {
    INT_STATUS old_status = disable_int();
    kprintf(KPL_DEBUG, "all: head->");
    for (
        list_node_t *p = task_all_list.head.next;
        p != &(task_all_list.tail);
        p = p->next
    ) {
        task_t *t = __list_node_struct(task_t, list_all_tag, p);
        kprintf(KPL_DEBUG, "{0x%X, %s}->", t, t->task_name);
    }
    kprintf(KPL_DEBUG, "tail\n");
    set_int_status(old_status);
}

void print_ready_tasks() {
    INT_STATUS old_status = disable_int();
    print_tasks(&task_ready_list);
    set_int_status(old_status);
}

void print_exit_tasks() {
    INT_STATUS old_status = disable_int();
    print_tasks(&task_exit_list);
    set_int_status(old_status);
}

void print_sleeping_tasks() {
    INT_STATUS old_status = disable_int();
    print_tasks(&sleeping_list);
    set_int_status(old_status);
}

void thread_join(task_t *task) {
    INT_STATUS old_status = disable_int();
    if (
        task == NULL || !list_find(&task_all_list, &(task->list_all_tag)) ||
        task->status == TASK_STOPPED || task->status == TASK_DEAD
    ) {
        set_int_status(old_status);
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
    ASSERT(!list_find(&task_ready_list, &(current_task->general_tag)));
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
    // INT_STATUS old_status = disable_int();
    // task_t *curr = current_task;
    // set_int_status(old_status);
    // return curr;
#ifdef KDEBUG
    INT_STATUS old_status = disable_int();
    uint32_t esp;
    __asm_volatile (
        "mov %0, esp"
        : "=g"(esp)
        :
        :
    );
    // kprintf(KPL_DEBUG, "esp=0x%X, current_task=0x%X\n", esp, current_task);
    ASSERT((esp & PG_START_ADDRESS_MASK) == (uintptr_t)current_task);
    set_int_status(old_status);
#endif
    return current_task;
}

// should be called every 1 msec
void sleep_manage() {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    for (
        list_node_t *p = sleeping_list.head.next;
        p != (&sleeping_list.tail);
    ) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        if (t->sleep_msec == 0) {
            list_node_t *victim = p;
            p = p->next;
            list_erase(victim);
            thread_unblock(t);
        } else {
            (t->sleep_msec)--;
            p = p->next;
        }
    }
}

static void __thread_yield() {
    ASSERT(current_task->status == TASK_RUNNING);
    schedule();
}

void thread_yield() {
    INT_STATUS old_status = disable_int();
    __thread_yield();
    set_int_status(old_status);
}

void thread_msleep(uint32_t msec) {
    INT_STATUS old_status = disable_int();
    if (msec == 0) {
        __thread_yield();
    }
    current_task->sleep_msec = msec;
    ASSERT(!list_find(&sleeping_list, &(current_task->general_tag)));
    list_push_front(&sleeping_list, &(current_task->general_tag));
    current_task->status = TASK_BLOCKED;
    schedule();
    set_int_status(old_status);
}
