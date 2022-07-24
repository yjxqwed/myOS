#include <sys/syscall.h>
#include <sys/isr.h>
#include <kprintf.h>
#include <device/tty.h>
#include <thread/thread.h>
#include <thread/process.h>
#include <common/types.h>
#include <common/debug.h>
#include <common/utils.h>
#include <mm/pmem.h>
#include <mm/vmm.h>

// #include <fs/myfs/fs.h>
#include <fs/simplefs/simplefs.h>

static void *sys_sleep(uint32_t ms);

typedef void * (*syscall_handler0_t)(void);
typedef void * (*syscall_handler1_t)(void *arg1);
typedef void * (*syscall_handler2_t)(void *arg1, void *arg2);
typedef void * (*syscall_handler3_t)(void *arg1, void *arg2, void *arg3);

static void *handlers[NUM_SYSCALL];

static void *syscall_handler(isrp_t *p) {
    if (p->eax < SYSCALL_ARG1) {
        return ((syscall_handler0_t)(handlers[p->eax]))();
    } else if (p->eax < SYSCALL_ARG2) {
        return ((syscall_handler1_t)(handlers[p->eax]))(p->ebx);
    } else if (p->eax < SYSCALL_ARG3) {
        return ((syscall_handler2_t)(handlers[p->eax]))(p->ebx, p->ecx);
    } else {
        return ((syscall_handler3_t)(handlers[p->eax]))(p->ebx, p->ecx, p->edx);
    }
}

static void *sys_clear() {
    clear_curr_console();
    return NULL;
}

void syscall_init() {
    // file system
    handlers[SYSCALL_OPEN] = sys_open;
    handlers[SYSCALL_CLOSE] = sys_close;
    handlers[SYSCALL_WRITE] = sys_write;
    handlers[SYSCALL_READ] = sys_read;
    handlers[SYSCALL_LSEEK] = sys_lseek;
    handlers[SYSCALL_UNLINK] = sys_unlink;
    handlers[SYSCALL_STAT] = sys_stat;
    handlers[SYSCALL_LIST_FILES] = sys_list_files;
    // memory
    handlers[SYSCALL_BRK] = sys_brk;
    handlers[SYSCALL_MM] = sys_mm;
    // sleep
    handlers[SYSCALL_SLEEP] = sys_sleep;
    // process
    handlers[SYSCALL_GETPID] = sys_getpid;
    handlers[SYSCALL_GETPPID] = sys_getppid;
    handlers[SYSCALL_CREATE_PROCESS] = sys_create_process;
    handlers[SYSCALL_PS] = sys_ps;
    handlers[SYSCALL_EXIT] = sys_exit;
    handlers[SYSCALL_WAIT] = sys_wait;
    // clear
    handlers[SYSCALL_CLS] = sys_clear;
    register_handler(0x80, syscall_handler);
}

static void *sys_sleep(uint32_t ms) {
    thread_msleep(ms);
    return NULL;
}
