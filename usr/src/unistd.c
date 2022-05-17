#include <sys/syscall.h>
#include <arch/x86.h>
#include <common/types.h>
#include <unistd.h>
#include <stdio.h>

#define _syscall0(no) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no) \
        : "memory" \
    ); \
    retval; \
})

#define _syscall1(no, arg1) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no), "b"(arg1) \
        : "memory" \
    ); \
    retval; \
})

#define _syscall3(no, arg1, arg2, arg3) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no), "b"(arg1), "c"(arg2), "d"(arg3) \
        : "memory" \
    ); \
    retval; \
})

// int getpid() {
//     return _syscall0(SYSCALL_GETPID);
// }

int write(int fd, const void *buffer, size_t count) {
    return _syscall3(SYSCALL_WRITE, fd, buffer, count);
}

int read(int fd, void *buffer, size_t count) {
    return _syscall3(SYSCALL_READ, fd, buffer, count);
}

static void *__brk(void *addr) {
    return (void *)_syscall1(SYSCALL_BRK, addr);
}

void *sbrk(intptr_t increment) {
    static void* old_brk = NULL;
    if (old_brk == NULL) {
        old_brk = __brk(NULL);
    }
    void *retval = old_brk;
    if (increment != 0) {
        void *new_brk = __brk((int32_t)old_brk + increment);
        if (new_brk != NULL) {
            old_brk = new_brk;
        } else {
            retval = (void *)-1;
        }
    }
    return retval;
}

int brk(void *addr) {
    if (addr == NULL || __brk(addr) == NULL) {
        return -1;
    }
    return 0;
}

void sleep(uint32_t ms) {
    _syscall1(SYSCALL_SLEEP, ms);
}
