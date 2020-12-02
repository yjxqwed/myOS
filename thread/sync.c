#include <thread/sync.h>

void sem_init(sem_t *sem, uint16_t val) {
    sem->val = val;
    list_init(&(sem->wait_list));
}

void sem_up(sem_t *sem) {
    
}

void sem_down(sem_t *sem);