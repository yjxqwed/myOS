#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define NUM_SYSCALL 32

enum {
    SYSCALL_ARG0 = 0,
    SYSCALL_GETPID,
    SYSCALL_ARG1,
    SYSCALL_WRITE,
    SYSCALL_BRK,
    SYSCALL_SBRK,
    SYSCALL_SLEEP,
    SYSCALL_ARG2,
    SYSCALL_ARG3
};

void syscall_init();

#endif