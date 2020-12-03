/*   myOS kernel main code   */

#include <kprintf.h>
#include <thread/thread.h>
#include <thread/sync.h>

static void test(void *args) {
    char *a = (char *)args;
    // for (int i = 0; i < 1000000 * 10; i++) {
    //     if (i % 1000000 == 0) {
    //         kprintf(KPL_DEBUG, " test: %s ", a);
    //         // print_ready_tasks();
    //         i = 0;
    //     }
    // }
    // kprintf(KPL_DEBUG, " test: %s ", a);
    for (int i = 0; i < 10; i++) {
        kprintf(KPL_DEBUG, " test: %s ", a);
    }
}

static void kmain(void *args) {
    kprintf(KPL_DEBUG, "main thread!\n");
    // thread_start("test1", 31, test, "abc");
    while (1);
}

static mutex_t m;
static int32_t x = 10000000;
static void test1(void *args) {
    char *name = (char *)args;
    // while (1) {
    //     kprintf(KPL_DUMP, "%s: x = %d", name, x);
    //     x++;
    // }
    uint32_t times = 0;
    mutex_lock(&m);
    while (x >= 0) {
        x--;
        times++;
    }
    mutex_unlock(&m);
    kprintf(KPL_DEBUG, "%s ends: times = %d\n", name, times);
}

static void test2(void *args) {
    char *name = (char *)args;
    // while (1) {
    //     kprintf(KPL_DEBUG, "%s: x = %d", name, x);
    //     x--;
    // }
    uint32_t times = 0;
    mutex_lock(&m);
    while (x < 20000000) {
        x++;
        times++;
    }
    mutex_unlock(&m);
    kprintf(KPL_DEBUG, "%s ends: times = %d\n", name, times);
}

static void parent(void *args) {
    kprintf(KPL_DEBUG, "parent start\n");
    mutex_init(&m);
    task_t *task1 = thread_start("test1", 5, test1, "test--");
    task_t *task2 = thread_start("test2", 5, test2, "test++");
    thread_join(task1);
    thread_join(task2);
    kprintf(KPL_DEBUG, "parent end\n");
}

void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    task_t *p = thread_start("parent", 10, parent, NULL);
    thread_join(p);
    while (1);
}
