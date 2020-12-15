#ifndef __SYNC_H__
#define __SYNC_H__

/**
 * @file sync.h
 * @brief thread synchronization primitives
 */

#include <common/types.h>
#include <thread/thread.h>
#include <list.h>

typedef struct Semaphore {
    uint16_t val;
    list_t wait_list;
} sem_t;

void sem_init(sem_t *sem, uint16_t val);
void sem_up(sem_t *sem);
void sem_down(sem_t *sem);

typedef struct Mutex {
    // sem_t sem;

    list_t wait_list;

    task_t *holder;
    //the number of times that holder repeatedly acquire this mutex
    uint16_t holder_repeat_nr;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

typedef struct ConditionVariable {

} cv_t;



#endif