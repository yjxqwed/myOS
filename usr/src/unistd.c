#include <sys/syscall.h>
#include <arch/x86.h>
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

#define _syscall2(no, arg1, arg2) ({ \
    int32_t retval; \
    __asm_volatile( \
        "int 0x80" \
        : "=a"(retval) \
        : "a"(no), "b"(arg1), "c"(arg2) \
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


int open(const char *filename, uint32_t flags) {
    return _syscall2(SYSCALL_OPEN, filename, flags);
}

int close(int fd) {
    return _syscall1(SYSCALL_CLOSE, fd);
}

int stat(const char *filename, stat_t *s) {
    return _syscall2(SYSCALL_STAT, filename, s);
}

int unlink(const char *filename) {
    return _syscall1(SYSCALL_UNLINK, filename);
}

int list_files(stat_t *s) {
    return _syscall1(SYSCALL_LIST_FILES, s);
}

off_t lseek(int fd, off_t offset, int whence) {
    return (off_t)_syscall3(SYSCALL_LSEEK, fd, offset, whence);
}

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

pid_t getpid() {
    return _syscall0(SYSCALL_GETPID);
}

pid_t getppid() {
    return _syscall0(SYSCALL_GETPPID);
}

pid_t create_process(const char *filename, char * const argv[]) {
    return _syscall2(SYSCALL_CREATE_PROCESS, filename, argv);
}

void clear() {
    _syscall0(SYSCALL_CLS); 
}

int ps(task_info_t *tis, size_t count) {
    return _syscall2(SYSCALL_PS, tis, count);
}

void _exit(int status) {
    _syscall1(SYSCALL_EXIT, status);
}

pid_t wait(int *status) {
    return _syscall1(SYSCALL_WAIT, status);
}
