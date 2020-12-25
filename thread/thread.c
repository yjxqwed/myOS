#include <thread/thread.h>
#include <string.h>
#include <arch/x86.h>
#include <sys/isr.h>
#include <mm/kvmm.h>
#include <common/debug.h>
#include <common/types.h>
#include <kprintf.h>
#include <sys/tss.h>

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

void task_push_back_ready(task_t *t) {
    __list_push_back(&task_ready_list, t, general_tag);
}

void task_push_back_all(task_t *t) {
    __list_push_back(&task_all_list, t, list_all_tag);
}

static void clear_exit_q();

static void thread_run_func(thread_func_t func, void *args) {
    enable_int();
    func(args);
    // when the function is done executed
    disable_int();
    clear_exit_q();
    current_task->status = TASK_FINISHED;

    __list_push_back(&task_exit_list, current_task, general_tag);

    while (!list_empty(&(current_task->join_list))) {
        task_t *t = __list_pop_front(&(current_task->join_list), task_t, general_tag);
        ASSERT(t->status == TASK_BLOCKED);
        t->status = TASK_READY;
        task_push_back_ready(t);
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

task_t *task_create(
    const char *name, uint16_t prio,
    thread_func_t func, void *args
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
    task->vmm = NULL;
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
    ASSERT(task->status == TASK_READY);
    task_push_back_ready(task);
    task_push_back_all(task);
    set_int_status(old_status);
    return task;
}

void thread_kmain() {
    kmain = (task_t *)reserved_pcb;
    task_init(kmain, "kmain", 31);
    INT_STATUS old_status = disable_int();
    task_push_back_all(kmain);
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

    while (!list_empty(&task_exit_list)) {
        task_t *t = __list_pop_front(&task_exit_list, task_t, general_tag);
        ASSERT(t->status == TASK_FINISHED);
        list_erase(&(t->list_all_tag));
        k_free_pages(t, 1);
    }
}

static void schedule() {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    task_t *old = current_task;
    ASSERT(!list_empty(&task_ready_list));

    task_t *next = __list_pop_front(&task_ready_list, task_t, general_tag);
    ASSERT(next->stack_guard == 0x19971125);
    ASSERT(next->status == TASK_READY);
    next->status = TASK_RUNNING;
    current_task = next;
    if (old != next) {
        if (next->vmm != NULL) {
            // if next is a user process, the kernel stack should always
            // be empty when in the user mode
            tss_update_esp0((uint32_t)next + PAGE_SIZE);
            load_page_dir(next->vmm->pgdir);
        } else {
            load_page_dir(NULL);
        }
        extern void switch_to(task_t *prev, task_t *next);
        switch_to(old, next);
    }
}

void time_scheduler() {
    ASSERT(current_task != NULL);
    ASSERT(current_task->stack_guard == 0x19971125);
    (current_task->elapsed_ticks)++;
    if (current_task->ticks == 0) {
        task_push_back_ready(current_task);
        current_task->status = TASK_READY;
        current_task->ticks = current_task->priority;
        schedule();
    } else {
        (current_task->ticks)--;
    }
}

static void print_tasks(const list_t *l) {
    char *name;
    if (l == &task_all_list) {
        return;
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
    list_node_t *p;
    __list_for_each(l, p) {
        task_t *t = __container_of(task_t, general_tag, p);
        kprintf(KPL_DEBUG, "{0x%X, %s}->", t, t->task_name);
    }
    kprintf(KPL_DEBUG, "tail\n");
}

void print_all_tasks() {
    INT_STATUS old_status = disable_int();
    kprintf(KPL_DEBUG, "all: head->");
    list_node_t *p;
    list_t *l = &task_all_list;
    __list_for_each(l, p) {
        task_t *t = __container_of(task_t, list_all_tag, p);
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
        task->status == TASK_FINISHED
    ) {
        set_int_status(old_status);
        return;
    }
    ASSERT(task != current_task);
    __list_push_back(&(task->join_list), current_task, general_tag);
    current_task->status = TASK_BLOCKED;
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
    task_push_back_ready(task);
    task->status = TASK_READY;
}

task_t *get_current_thread() {
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
        task_t *t = __container_of(task_t, general_tag, p);
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
    __list_push_back(&sleeping_list, current_task, general_tag);
    current_task->status = TASK_BLOCKED;
    schedule();
    set_int_status(old_status);
}
