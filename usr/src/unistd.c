#include <sys/syscall.h>
#include <arch/x86.h>
#include <common/types.h>
#include <unistd.h>

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

void *brk(uintptr_t __addr) {
    return _syscall1(SYSCALL_BRK, __addr);
}

void sleep(uint32_t ms) {
    return _syscall1(SYSCALL_SLEEP, ms);
}
