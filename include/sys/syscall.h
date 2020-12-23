#ifndef __SYSCALL_H__
#define __SYSCALL_H__

enum {
    SYSCALL_GETPID,
    SYSCALL_WRITE,
    SYSCALL_BRK,
    SYSCALL_SBRK,
    NUM_SYSCALL
};

void syscall_init();

#endif