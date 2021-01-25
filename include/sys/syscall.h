#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define NUM_SYSCALL 32

#define SYSCALL_FAIL ((void *)-1)

enum {
    SYSCALL_ARG0 = 0,

    SYSCALL_GETPID,

    SYSCALL_ARG1,


    SYSCALL_BRK,
    // SYSCALL_SBRK,
    SYSCALL_SLEEP,

    SYSCALL_ARG2,

    SYSCALL_ARG3,
    SYSCALL_READ,
    SYSCALL_WRITE,
};

void syscall_init();

#endif
