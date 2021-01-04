#ifndef __SYNC_H__
#define __SYNC_H__

/**
 * @file sync.h
 * @brief thread synchronization primitives
 */

typedef struct Semaphore sem_t;
typedef struct Mutex mutex_t;

#include <common/types.h>
#include <thread/thread.h>
#include <list.h>

struct Semaphore {
    list_t wait_list;
    uint16_t val;
};

void sem_init(sem_t *sem, uint16_t val);
void sem_up(sem_t *sem);
void sem_down(sem_t *sem);

struct Mutex {
    list_t wait_list;
    task_t *holder;
    //the number of times that holder repeatedly acquire this mutex
    uint16_t holder_repeat_nr;
};

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

typedef struct ConditionVariable {

} cv_t;



#endif