#include <fs/simplefs/simplefs.h>

#include <lib/kprintf.h>

void kernel_test_simplefs() {
    // int fd1 = sys_open("file1", 0);
    // kprintf(KPL_DEBUG, "fd1=%d\n", fd1);
    // int fd2 = sys_open("file2", O_CREAT);
    // kprintf(KPL_DEBUG, "fd2=%d\n", fd2);

    stat_t *buffer = kmalloc(4096 * sizeof(stat_t));
    int nfiles = simplefs_list_files(buffer);
    kprintf(KPL_DEBUG, "simplefs_list_files: %d\n", nfiles);
    for (int i = 0; i < nfiles; i++) {
        stat_t *sb = buffer + i;
        kprintf(KPL_DEBUG, "%d, %d, %d, %s\n", sb->file_id, sb->size, sb->blocks, sb->filename);
    }

    int fd = simplefs_file_open("simplefs.c", 0);
    kprintf(KPL_DEBUG, "open simplefs.c: %d\n", fd);
    if (fd != -1) {
        int nread = simplefs_file_read(fd, buffer, 512);
        ((char *)buffer)[nread] = '\0';
        kprintf(KPL_DEBUG, "%s", (char *)buffer);
    }
    kfree(buffer);
}