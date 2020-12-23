#ifndef __SYSCALL_H__
#define __SYSCALL_H__

enum {
    SYSCALL_GETPID,
    SYSCALL_WRITE,
    NUM_SYSCALL
};

void syscall_init();

#endif