/*   myOS kernel main code   */

#include <kprintf.h>
#include <thread/thread.h>
#include <thread/sync.h>
#include <mm/kvmm.h>
#include <common/debug.h>

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

static void parent1(void *args) {
    uint32_t interval = *(uint32_t *)args;
    kprintf(KPL_DEBUG, "args=0x%x\n", args);
    int x = 0;
    kprintf(KPL_DEBUG, "parent(%d) start\n", interval);
    MAGICBP;
    // mutex_init(&m);
    // task_t *task1 = thread_start("supplier", 5, test1, "supplier");
    // task_t *task2 = thread_start("consumer1", 5, test2, "consumer1");
    // task_t *task3 = thread_start("consumer2", 5, test2, "consumer2");
    // thread_join(task1);
    // thread_join(task2);
    // thread_join(task3);
    // thread_msleep(5000);
    while (x < 100) {
        kprintf(KPL_DUMP, "parent(%d): x=%d\n", interval, x);
        thread_msleep(interval);
        x++;
    }
    kprintf(KPL_DEBUG, "parent(%d) end\n", interval);
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

static void parent(void *args) {
    uint32_t itvl1 = 2000, itvl2 = 3000;
    kprintf(KPL_DEBUG, "&itvl1=0x%x, itvl1=%d\n", &itvl1, itvl1);
    task_t *p1 = thread_start("parent1", 30, parent1, &itvl1);
    thread_msleep(5000);
    kprintf(KPL_DEBUG, "&itvl1=0x%x, itvl1=%d\n", &itvl1, itvl1);
    thread_join(p1);
}

static void test_thread() {
    task_t *p2 = thread_start("parent", 30, parent, NULL);
    // // thread_join(p1);
    // // thread_join(p2);
    // kprintf(KPL_DEBUG, "&itvl1=0x%x, itvl1=%d\n", &itvl1, itvl1);
    // task_t *p1 = thread_start("parent1", 30, parent1, &itvl1);
    // // thread_msleep(5000);
    // kprintf(KPL_DEBUG, "&itvl1=0x%x, itvl1=%d\n", &itvl1, itvl1);
    // // thread_join(p1);
}

static void test_kmalloc() {
    char *b0 = kmalloc(20);
    // kprintf(KPL_DEBUG, "b0 = 0x%X\n", b0);
    // kfree(b0);

    char *b1 = kmalloc(33);
    // kprintf(KPL_DEBUG, "b1 = 0x%X\n", b1);
    char *b2 = kmalloc(10);
    // kprintf(KPL_DEBUG, "b2 = 0x%X\n", b2);
    char *b3 = kmalloc(1025);
    // kprintf(KPL_DEBUG, "b3 = 0x%X\n", b3);
    char *b4 = kmalloc(9999);
    // kprintf(KPL_DEBUG, "b4 = 0x%X\n", b4);
    // kfree(b4);
    char *b5 = kmalloc(31);
    // kprintf(KPL_DEBUG, "b5 = 0x%X\n", b5);
    // kfree(b5);
    char *b6 = kmalloc(63);
    // kprintf(KPL_DEBUG, "b6 = 0x%X\n", b6);
    // kfree(b6);
    char *b7 = kmalloc(127);
    // kprintf(KPL_DEBUG, "b7 = 0x%X\n", b7);
    // kfree(b7);
    kfree(b0);
    kfree(b1);
    kfree(b2);
    kfree(b3);
    kfree(b4);
    kfree(b5);
    kfree(b6);
    kfree(b7);
}

static void test_kmalloc1() {
    uint32_t a = 20;
    char *b0 = kmalloc(a);
    a *= 2;
    char *b1 = kmalloc(a);
    a *= 2;
    char *b2 = kmalloc(a);
    a *= 2;
    char *b3 = kmalloc(a);
    a *= 2;
    char *b4 = kmalloc(a);
    a *= 2;
    char *b5 = kmalloc(a);
    a *= 2;
    char *b6 = kmalloc(a);
    a *= 2;
    char *b7 = kmalloc(a);
    kfree(b0);
    kfree(b1);
    kfree(b2);
    kfree(b3);
    kfree(b4);
    kfree(b5);
    kfree(b6);
    kfree(b7);
}

static void test(void *args) {
    int id = *(int *)args;
    kprintf(KPL_DEBUG, "test%d start\n", id);
    for (int i = 0; i < 1000; i++) {
        test_kmalloc();
        test_kmalloc1();
    }
    kprintf(KPL_DEBUG, "test%d end\n", id);
    // while (1);
}

static void test_parent(void *args) {
    // task_t *tasks[100];
    // for (int i = 0; i < 10; i++) {
    //     tasks[i] = thread_start("test", 15, test, );
    // }
    vmm_print();
    int id1 = 1, id2 = 2;
    task_t *t1 = thread_start("test1", 30, test, &id1);
    task_t *t2 = thread_start("test2", 30, test, &id2);
    thread_join(t1);
    thread_join(t2);
    vmm_print();
    kprintf(KPL_DEBUG, "test_parent end.\n");
    // while (1);
}


void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // test_thread();
    // test_k_get_free_page();
    // test_kmalloc();
    thread_start("test_parent", 30, test_parent, NULL);
    while (1);
}
