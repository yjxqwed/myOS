#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define NUM_SYSCALL 32

#define SYSCALL_FAIL ((void *)-1)

enum {
    SYSCALL_ARG0 = 0,  // syscalls after this require 0 arguments
    SYSCALL_GETPID,
    SYSCALL_GETPPID,
    SYSCALL_CLS,

    SYSCALL_ARG1,      // syscalls after this require 1 argument
    SYSCALL_BRK,
    SYSCALL_SLEEP,
    SYSCALL_CLOSE,
    SYSCALL_UNLINK,
    SYSCALL_LIST_FILES,
    SYSCALL_EXIT,
    SYSCALL_WAIT,

    SYSCALL_ARG2,      // syscalls after this require 2 arguments
    SYSCALL_OPEN,
    SYSCALL_STAT,
    SYSCALL_CREATE_PROCESS,
    SYSCALL_PS,

    SYSCALL_ARG3,      // syscalls after this require 3 arguments
    SYSCALL_READ,
    SYSCALL_WRITE,
    SYSCALL_LSEEK,

};

void syscall_init();

#endif
