#include <sys/syscall.h>
#include <arch/x86.h>
#include <common/types.h>
#include <unistd.h>

#define _syscall0(no) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no)\
        : \
    ); \
    retval; \
})

#define _syscall1(no, arg1) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no), "b"(arg1) \
        : \
    ); \
    retval; \
})

// int getpid() {
//     return _syscall0(SYSCALL_GETPID);
// }

int write(const char* str) {
    return _syscall1(SYSCALL_WRITE, str);
}

void *sbrk(intptr_t __delta) {
    return _syscall1(SYSCALL_SBRK, __delta);
}