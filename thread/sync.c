#include <thread/sync.h>
#include <thread/thread.h>
#include <arch/x86.h>
#include <common/debug.h>
#include <kprintf.h>
#include <myos.h>

void sem_init(sem_t *sem, uint16_t val) {
    sem->val = val;
    list_init(&(sem->wait_list));
}

void sem_up(sem_t *sem) {
    ASSERT(sem != NULL);
    INT_STATUS old_status = disable_int();
    (sem->val)++;
    // list_node_t *p = list_pop_front(&(sem->wait_list));
    // if (p != NULL) {
    //     task_t *t = __container_of(task_t, general_tag, p);
    //     thread_unblock(t);
    // }
    task_t *t = __list_pop_front(&(sem->wait_list), task_t, general_tag);
    if (t != NULL) {
        thread_unblock(t);
    }
    set_int_status(old_status);
}

void sem_down(sem_t *sem) {
    ASSERT(sem != NULL);
    INT_STATUS old_status = disable_int();
    while (sem->val == 0) {
        task_t *curr = get_current_thread();
        // if (list_find(&(sem->wait_list), &(curr->general_tag))) {
        //     kprintf(KPL_DEBUG, "mutex = 0x%X\n", __container_of(mutex_t, sem, sem));
        //     kprintf(KPL_DEBUG, "task = 0x%X\n", curr);
        // }
        // ASSERT(!list_find(&(sem->wait_list), &(curr->general_tag)));
        // list_push_back(&(sem->wait_list), &(curr->general_tag));
        __list_push_back(&(sem->wait_list), curr, general_tag);
        thread_block_self(TASK_BLOCKED);
    }
    (sem->val)--;
    set_int_status(old_status);
}


void mutex_init(mutex_t *mutex) {
    list_init(&(mutex->wait_list));
    mutex->holder = NULL;
    mutex->holder_repeat_nr = 0;
}

static void print_mutex(mutex_t *mutex) {
    ASSERT(get_int_status() == INTERRUPT_OFF);
    kprintf(KPL_DEBUG, "mutex=(0x%X){", mutex);
    if (mutex->holder == NULL) {
        kprintf(KPL_DEBUG, "holder=NULL,");
    } else {
        kprintf(KPL_DEBUG, "holder=0x%X,", mutex->holder);
    }
    kprintf(KPL_DEBUG, "repeat_nr=%d}", mutex->holder_repeat_nr);
}

void mutex_lock(mutex_t *mutex) {
    // ASSERT(__valid_kva(mutex));
    INT_STATUS old_status = disable_int();
    task_t *curr = get_current_thread();
    // kprintf(KPL_DEBUG, "lock: curr=%s, ", curr->task_name);
    // print_mutex(mutex);
    if (mutex->holder == curr) {
        (mutex->holder_repeat_nr)++;
        set_int_status(old_status);
        return;
    }
    while (mutex->holder != NULL) {
        // kprintf(KPL_NOTICE, "{curr(%s)->status(%d)}", curr->task_name, curr->status);
        ASSERT(curr->status == TASK_RUNNING);
        // ASSERT(!list_find(&(mutex->wait_list), &(curr->general_tag)));
        // list_push_back(&(mutex->wait_list), &(curr->general_tag));
        __list_push_back(&(mutex->wait_list), curr, general_tag);
        thread_block_self(TASK_BLOCKED);
    }
    ASSERT(mutex->holder == NULL);
    ASSERT(mutex->holder_repeat_nr == 0);
    mutex->holder = curr;
    mutex->holder_repeat_nr = 1;
    set_int_status(old_status);
}

void mutex_unlock(mutex_t *mutex) {
    // ASSERT(mutex != NULL);
    ASSERT(__valid_kva(mutex));
    INT_STATUS old_status = disable_int();
    // kprintf(KPL_DEBUG, "mutex=0x%X\n", mutex);
    task_t *curr = get_current_thread();
    // kprintf(KPL_DEBUG, "unlock: curr=%s, ", curr->task_name);
    // print_mutex(mutex);
    ASSERT(mutex->holder != NULL);
    // if (mutex->holder != curr) {
    //     print_mutex(mutex);
    //     kprintf(KPL_PANIC, "curr=0x%X\n", curr);
    // }
    ASSERT(mutex->holder == curr);
    ASSERT(mutex->holder_repeat_nr > 0);
    (mutex->holder_repeat_nr)--;
    if (mutex->holder_repeat_nr == 0) {
        mutex->holder = NULL;
        // list_node_t *p = list_pop_front(&(mutex->wait_list));
        // if (p != NULL) {
        //     task_t *t = __container_of(task_t, general_tag, p);
        //     thread_unblock(t);
        // }
        task_t *t = __list_pop_front(&(mutex->wait_list), task_t, general_tag);
        if (t != NULL) {
            thread_unblock(t);
        }
    }
    set_int_status(old_status);
}
