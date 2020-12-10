#include <thread/sync.h>
#include <thread/thread.h>
#include <arch/x86.h>
#include <common/debug.h>

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
}

void mutex_unlock(mutex_t *mutex) {
    ASSERT(mutex->holder == get_current_thread());
    if (mutex->holder_repeat_nr > 1) {
        (mutex->holder_repeat_nr)--;
    } else {
        ASSERT(mutex->holder_repeat_nr == 1);
        mutex->holder_repeat_nr = 0;
        mutex->holder = NULL;
        sem_up(&(mutex->sem));
    }
}
