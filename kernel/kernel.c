/*   myOS kernel main code   */

#include <kprintf.h>
#include <thread/thread.h>
#include <thread/sync.h>
#include <mm/vmm.h>
#include <common/debug.h>

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
static int32_t x = 5;
static void test1(void *args) {
    char *name = (char *)args;
    while (1) {
        mutex_lock(&m);
        if (x < 5) {
            x++;
            kprintf(KPL_DUMP, "%s: x = %d\n", name, x);
        }
        mutex_unlock(&m);
    }
}

static void test2(void *args) {
    char *name = (char *)args;
    // while (1) {
    //     kprintf(KPL_DEBUG, "%s: x = %d", name, x);
    //     x--;
    // }
    // uint32_t times = 0;
    // mutex_lock(&m);
    // while (x < 20000000) {
    //     x++;
    //     times++;
    // }
    // mutex_unlock(&m);
    // kprintf(KPL_DEBUG, "%s ends: times = %d\n", name, times);
    while (1) {
        mutex_lock(&m);
        if (x > 0) {
            x--;
            kprintf(KPL_DUMP, "%s: x = %d\n", name, x);
        }
        mutex_unlock(&m);
    }
}

static void parent(void *args) {
    kprintf(KPL_DEBUG, "parent start\n");
    mutex_init(&m);
    task_t *task1 = thread_start("supplier", 5, test1, "supplier");
    task_t *task2 = thread_start("consumer1", 5, test2, "consumer1");
    task_t *task3 = thread_start("consumer2", 5, test2, "consumer2");
    thread_join(task1);
    thread_join(task2);
    thread_join(task3);
    kprintf(KPL_DEBUG, "parent end\n");
}

static void test_k_get_free_page() {
    uint32_t *p1 = k_get_free_pages(15, GFP_ZERO);
    kprintf(KPL_DUMP, "p1 = 0x%X\n", p1);
    *p1 = 0x12345678;
    MAGICBP;
    uint32_t *p2 = k_get_free_pages(1, GFP_ZERO);
    kprintf(KPL_DUMP, "p2 = 0x%X\n", p2);
    k_free_pages(p1, 15);
    uint32_t *p3 = k_get_free_pages(1, GFP_ZERO);
    kprintf(KPL_DUMP, "p3 = 0x%X\n", p3);
    uint32_t *p4 = k_get_free_pages(15, GFP_ZERO);
    kprintf(KPL_DUMP, "p4 = 0x%X\n", p4);
    k_free_pages(p3, 1);
    uint32_t *p5 = k_get_free_pages(15, GFP_ZERO);
    kprintf(KPL_DUMP, "p5 = 0x%X\n", p5);
    MAGICBP;
    *p5 = 0x98765432;
}

static void test_thread() {
    task_t *p = thread_start("parent", 10, parent, NULL);
    thread_join(p);
}

void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // test_thread();
    // test_k_get_free_page();
    while (1);
}
