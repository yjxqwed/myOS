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
    mutex_lock(&m);
    kprintf(KPL_DEBUG, " test%d start ", id);
    mutex_unlock(&m);
    // for (int i = 0; i < 1000; i++) {
    //     test_kmalloc();
    //     test_kmalloc1();
    // }
    // uint32_t slp = (id * 10000) % 10007;
    // mutex_lock(&m);
    // kprintf(KPL_DEBUG, " test%d will sleep for %d ms ", id, slp);
    // mutex_unlock(&m);
    // thread_msleep(slp);
    for (int i = 0; i < 1000000; i++);
    if (id % 17 == 0) {
        thread_yield();
    }
    for (int i = 0; i < 1000000; i++);
    mutex_lock(&m);
    kprintf(KPL_DEBUG, " test%d end ", id);
    mutex_unlock(&m);
    // while (1);
}

// sem_t sem;

static void test_yield(void *args) {

}

static void test_parent(void *args) {
    mutex_lock(&m);
    // // vmm_print();
    const char *me = get_current_thread()->task_name;
    kprintf(KPL_DEBUG, "me = %s\n", me);
    print_all_tasks();
    print_ready_tasks();
    print_sleeping_tasks();
    print_exit_tasks();
    kprintf(KPL_DEBUG, "=========\n", me);
    mutex_unlock(&m);
    // return;
    // while (1);
    // MAGICBP;
    int num_threads = 1000;
    task_t **tasks = (task_t **)kmalloc(sizeof(task_t *) * num_threads);
    int *ids = kmalloc(sizeof(int) * num_threads);
    
    for (int i = 0; i < num_threads; i++) {
        ids[i] = i;
        tasks[i] = thread_start("test", 30, test, &(ids[i]));
    }
    for (int i = 0; i < num_threads; i++) {
        thread_join(tasks[i]);
    }
    kfree(tasks);
    kfree(ids);
    // int id1 = 1, id2 = 2;
    // task_t *t1 = thread_start("test1", 30, test, &id1);
    // task_t *t2 = thread_start("test2", 30, test, &id2);
    // thread_join(t1);
    // thread_join(t2);
    mutex_lock(&m);
    // vmm_print();
    kprintf(KPL_DEBUG, "test_parent end.\n");
    mutex_unlock(&m);
    print_all_tasks();
    print_ready_tasks();
    print_sleeping_tasks();
    print_exit_tasks();
    // while (1);
}


void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // test_thread();
    // test_k_get_free_page();
    // test_kmalloc();
    print_all_tasks();
    print_ready_tasks();
    // print_sleeping_tasks();
    // print_exit_tasks();
    // MAGICBP;
    // for (int i = 0; i < 10000000; i++);
    // for (int i = 0; i < 10000000; i++);
    // for (int i = 0; i < 10000000; i++);
    // for (int i = 0; i < 10000000; i++);
    // print_all_tasks();
    // print_ready_tasks();
    // MAGICBP;
    mutex_init(&m);
    task_t *tp0 = thread_start("tp0", 30, test_parent, NULL);
    task_t *tp1 = thread_start("tp1", 30, test_parent, NULL);
    thread_join(tp0);
    thread_join(tp1);
    kprintf(KPL_DEBUG, "back to main\n");
    print_all_tasks();
    print_ready_tasks();
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    kprintf(KPL_DEBUG, "later\n");
    print_all_tasks();
    print_ready_tasks();
    thread_start("tp1", 30, test_parent, NULL);
    // thread_start("tp2", 30, test_parent, NULL);
    // thread_start("tp3", 30, test_parent, NULL);
    // thread_start("tp4", 30, test_parent, NULL);
    // thread_start("tp5", 30, test_parent, NULL);
    // thread_start("tp6", 30, test_parent, NULL);
    // thread_start("tp7", 30, test_parent, NULL);
    // thread_start("tp8", 30, test_parent, NULL);
    // thread_start("tp9", 30, test_parent, NULL);
    // thread_start("tp10", 30, test_parent, NULL);
    // thread_start("tp11", 30, test_parent, NULL);
    // thread_start("tp12", 30, test_parent, NULL);
    // thread_start("tp13", 30, test_parent, NULL);
    // thread_start("tp14", 30, test_parent, NULL);
    // thread_start("tp15", 30, test_parent, NULL);
    while (1);
}
