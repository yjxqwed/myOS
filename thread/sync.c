#include <thread/sync.h>
#include <thread/thread.h>
#include <arch/x86.h>
#include <common/debug.h>
#include <kprintf.h>

void sem_init(sem_t *sem, uint16_t val) {
    sem->val = val;
    list_init(&(sem->wait_list));
}

void sem_up(sem_t *sem) {
    INT_STATUS old_status = disable_int();
    (sem->val)++;
    list_node_t *p = list_pop_front(&(sem->wait_list));
    if (p != NULL) {
        task_t *t = __list_node_struct(task_t, general_tag, p);
        thread_unblock(t);
    }
    set_int_status(old_status);
}

void sem_down(sem_t *sem) {
    INT_STATUS old_status = disable_int();
    while (sem->val == 0) {
        task_t *curr = get_current_thread();
        ASSERT(!list_find(&(sem->wait_list), &(curr->general_tag)));
        list_push_back(&(sem->wait_list), &(curr->general_tag));
        thread_block_self(TASK_BLOCKED);
    }
    (sem->val)--;
    set_int_status(old_status);
}

void mutex_init(mutex_t *mutex) {
    sem_init(&(mutex->sem), 1);
    mutex->holder = NULL;
    mutex->holder_repeat_nr = 0;
}

void mutex_lock(mutex_t *mutex) {
    if (mutex->holder != get_current_thread()) {
        sem_down(&(mutex->sem));
        ASSERT(mutex->holder == NULL && mutex->holder_repeat_nr == 0);
        mutex->holder = get_current_thread();
        mutex->holder_repeat_nr = 1;
    } else {
        (mutex->holder_repeat_nr)++;
    }
    // INT_STATUS old_status = disable_int();
    // kprintf(KPL_DEBUG, "(mutex lock 0x%X)", mutex);
    // set_int_status(old_status);
}

void mutex_unlock(mutex_t *mutex) {
    // kprintf(KPL_DEBUG, "mutex=0x%X\n", mutex);
    // INT_STATUS old_status = disable_int();
    // if (mutex->holder != get_current_thread()) {
    //     kprintf(KPL_DEBUG, "holder=0x%X, curr=0x%X\n", mutex->holder, get_current_thread());
    // }
    // set_int_status(old_status);
    ASSERT(mutex->holder != NULL);
    ASSERT(mutex->holder == get_current_thread());
    // INT_STATUS old_status = disable_int();
    // kprintf(KPL_DEBUG, "(mutex lock 0x%X)", mutex);
    // set_int_status(old_status);
    if (mutex->holder_repeat_nr > 1) {
        (mutex->holder_repeat_nr)--;
    } else {
        ASSERT(mutex->holder_repeat_nr == 1);
        mutex->holder_repeat_nr = 0;
        mutex->holder = NULL;
        sem_up(&(mutex->sem));
    }
}
