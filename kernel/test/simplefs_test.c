#include <fs/simplefs/simplefs.h>

#include <lib/kprintf.h>

void kernel_test_simplefs() {
    int fd1 = sys_open("file1", 0);
    kprintf(KPL_DEBUG, "fd1=%d\n", fd1);
    int fd2 = sys_open("file2", O_CREAT);
    kprintf(KPL_DEBUG, "fd2=%d\n", fd2);
}