#include <fs/simplefs/simplefs.h>

#include <lib/kprintf.h>
#include <lib/string.h>

#include <thread/thread.h>
#include <thread/process.h>
#include <thread/sync.h>

#include <mm/kvmm.h>

#include <common/debug.h>



static void print2file(const void *args) {
    uint32_t id = (uint32_t)args;
    char s[10];
    ksprintf(s, "%x", id);
    int ls = strlen(s);

    int fd = simplefs_file_open("hello", 0);
    kprintf(KPL_DEBUG, "%x: fd=%d\n", id, fd);
    if (fd >= 0) {
        for (int i = 0; i < 10; i++) {
            int c = simplefs_file_write(fd, s, ls);
            kprintf(KPL_DEBUG, "%x: c=%d\n", id, c);
            // thread_msleep(500);
        }
        int ok = simplefs_file_close(fd);
        kprintf(KPL_DEBUG, "%x: ok=%d\n", id, ok);
    }

}

static void test_simplefs_concurrent() {
    task_t *tasks[20];
    // int *ids = (int *)k_get_free_pages(2, GFP_ZERO);
    for (int i = 0; i < 20; i++) {
        // ids[i] = i;
        tasks[i] = thread_start("print2file", 1, print2file, 0x1000 + i);
    }

    for (int i = 0; i < 20; i++) {
        // ids[i] = i;
        thread_join(tasks[i]);
    }
}

void kernel_test_simplefs() {

    test_simplefs_concurrent();

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