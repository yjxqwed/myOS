/*   myOS kernel main code   */

#include <kprintf.h>
#include <thread/thread.h>
#include <thread/sync.h>
#include <mm/kvmm.h>
#include <common/debug.h>
#include <thread/process.h>
// #include <usr/include/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

static void test_k_get_free_pages() {
    // pmem_print();
    // uint32_t *p1 = k_get_free_pages(15, GFP_ZERO);
    uint32_t *p1 = k_get_free_pages(15, GFP_ZERO);
    // mutex_lock(&m);
    // kprintf(KPL_DUMP, "p1 = 0x%X\n", p1);
    // mutex_unlock(&m);
    *p1 = 0x12345678;
    uint32_t *p2 = k_get_free_pages(1, GFP_ZERO);
    // mutex_lock(&m);
    // kprintf(KPL_DUMP, "p2 = 0x%X\n", p2);
    // mutex_unlock(&m);
    // k_free_pages(p1, 15);
    k_free_pages(p1, 15);
    uint32_t *p3 = k_get_free_pages(1, GFP_ZERO);
    // mutex_lock(&m);
    // kprintf(KPL_DUMP, "p3 = 0x%X\n", p3);
    // mutex_unlock(&m);
    // uint32_t *p4 = k_get_free_pages(15, GFP_ZERO);
    uint32_t *p4 = k_get_free_pages(15, GFP_ZERO);
    // mutex_lock(&m);
    // kprintf(KPL_DUMP, "p4 = 0x%X\n", p4);
    // mutex_unlock(&m);
    k_free_pages(p3, 1);
    // uint32_t *p5 = k_get_free_pages(15, GFP_ZERO);
    uint32_t *p5 = k_get_free_pages(15, GFP_ZERO);
    // mutex_lock(&m);
    // kprintf(KPL_DUMP, "p5 = 0x%X\n", p5);
    // mutex_unlock(&m);
    *p5 = 0x98765432;
    k_free_pages(p2, 1);
    // k_free_pages(p4, 15);
    // k_free_pages(p5, 15);
    k_free_pages(p4, 15);
    k_free_pages(p5, 15);
    // pmem_print();
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

// static void test_page_alloc() {
//     ppage_t *p1 = page_alloc(GFP_ZERO);
//     ppage_t *p2 = page_alloc(GFP_ZERO);
//     ppage_t *p3 = page_alloc(GFP_ZERO);
//     ppage_t *p4 = pages_alloc(12, )
// }

static void test_kmalloc() {
    uint32_t *b0 = kmalloc(20);
    *b0 = 0x12345678;
    uint32_t *b1 = kmalloc(33);
    *b1 = 0x12345678;
    uint32_t *b2 = kmalloc(10);
    *b2 = 0x12345678;
    uint32_t *b3 = kmalloc(1025);
    *b3 = 0x12345678;
    uint32_t *b4 = kmalloc(9999);
    *b4 = 0x12345678;
    uint32_t *b5 = kmalloc(31);
    *b5 = 0x12345678;
    uint32_t *b6 = kmalloc(63);
    *b6 = 0x12345678;
    uint32_t *b7 = kmalloc(127);
    *b7 = 0x12345678;
    // return;
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
    uint32_t *b0 = kmalloc(a);
    *b0 = 0x12345678;
    a *= 2;
    uint32_t *b1 = kmalloc(a);
    *b1 = 0x12345678;
    a *= 2;
    uint32_t *b2 = kmalloc(a);
    *b2 = 0x12345678;
    a *= 2;
    uint32_t *b3 = kmalloc(a);
    *b3 = 0x12345678;
    a *= 2;
    uint32_t *b4 = kmalloc(a);
    *b4 = 0x12345678;
    a *= 2;
    uint32_t *b5 = kmalloc(a);
    *b5 = 0x12345678;
    a *= 2;
    uint32_t *b6 = kmalloc(a);
    *b6 = 0x12345678;
    a *= 2;
    uint32_t *b7 = kmalloc(a);
    *b7 = 0x12345678;
    // return;
    kfree(b0);
    kfree(b1);
    kfree(b2);
    kfree(b3);
    kfree(b4);
    kfree(b5);
    kfree(b6);
    kfree(b7);
}

static void test_kmalloc2() {
    uint32_t *b4 = kmalloc(1023);
    *b4 = 0x12345678;
    INT_STATUS old_status = disable_int();
    kprintf(KPL_DEBUG, "b4=0x%X\n", b4);
    set_int_status(old_status);
    kfree(b4);
}

static void test_page_alloc() {
    ppage_t *p0 = pages_alloc(1, GFP_ZERO);
    ppage_t *p1 = pages_alloc(1, GFP_ZERO);
    ppage_t *p2 = pages_alloc(1, GFP_ZERO);
    page_incref(p0);
    page_incref(p1);
    ppage_t *p3 = pages_alloc(1, GFP_ZERO);
    page_incref(p2);
    page_decref(p0);
    page_incref(p3);
    page_decref(p1);
    page_decref(p2);
    page_decref(p3);
}

ppage_t *pp;

static void test_page_ref() {
    for (int i = 0; i < 10; i++) {
        page_incref(pp);
    }
    for (int i = 0; i < 10; i++) {
        page_decref(pp);
    }
}

static int num_thread_started = 0;
// mutex_t num_thread_started_lock;

static void test(void *args) {
    // mutex_lock(&num_thread_started_lock);
    INT_STATUS old_status = disable_int();
    num_thread_started++;
    kprintf(KPL_NOTICE, "num_thread_started=%d\n", num_thread_started);
    // pmem_print();
    set_int_status(old_status);
    // mutex_unlock(&num_thread_started_lock);
    // int id = (int)args;
    // mutex_lock(&m);
    // kprintf(KPL_DEBUG, " test%d start ", id);
    // mutex_unlock(&m);
    // for (int i = 0; i < 1000; i++) {
    //     test_kmalloc();
    //     test_kmalloc1();
    // }
    // uint32_t slp = (id * 10000) % 10007;
    // mutex_lock(&m);
    // kprintf(KPL_DEBUG, " test%d will sleep for %d ms ", id, slp);
    // mutex_unlock(&m);
    // thread_msleep(slp);
    // for (int i = 0; i < 1000000; i++);
    test_kmalloc();
    // if (id % 17 == 0) {
    //     thread_yield();
    // }
    test_kmalloc1();
    test_kmalloc2();
    // test_k_get_free_pages();
    // test_page_alloc();
    // test_page_ref();
    // for (int i = 0; i < 1000000; i++);
    // mutex_lock(&m);
    // kprintf(KPL_DEBUG, " test%d end ", id);
    // mutex_unlock(&m);
    // while (1);
}

// sem_t sem;

static void test_yield(void *args) {

}

static void test_parent(void *args) {
    // mutex_lock(&m);
    // // // vmm_print();
    // const char *me = get_current_thread()->task_name;
    // kprintf(KPL_DEBUG, "me = %s\n", me);
    // print_all_tasks();
    // print_ready_tasks();
    // print_sleeping_tasks();
    // print_exit_tasks();
    // kprintf(KPL_DEBUG, "=========\n", me);
    // mutex_unlock(&m);
    // return;
    // while (1);
    // MAGICBP;
    int num_threads = 200;
    // task_t **tasks = (task_t **)kmalloc(sizeof(task_t *) * num_threads);
    // int *ids = kmalloc(sizeof(int) * num_threads);

    // task_t **tasks = (task_t **)k_get_free_pages(2, GFP_ZERO);
    task_t *tasks[200];
    // int *ids = (int *)k_get_free_pages(2, GFP_ZERO);
    for (int i = 0; i < num_threads; i++) {
        // ids[i] = i;
        tasks[i] = thread_start("test", 1, test, i);
    }
    for (int i = 0; i < num_threads; i++) {
        thread_join(tasks[i]);
    }
    // k_free_pages(tasks, 2);
    // k_free_pages(ids, 2);
    // kfree(tasks);
    // kfree(ids);
    // int id1 = 1, id2 = 2;
    // task_t *t1 = thread_start("test1", 30, test, &id1);
    // task_t *t2 = thread_start("test2", 30, test, &id2);
    // thread_join(t1);
    // thread_join(t2);
    // mutex_lock(&m);
    // // vmm_print();
    // kprintf(KPL_DEBUG, "test_parent end.\n");
    // mutex_unlock(&m);
    // print_all_tasks();
    // print_ready_tasks();
    // print_sleeping_tasks();
    // print_exit_tasks();
    // while (1);
}

static void test_thread_kmalloc() {
    // print_all_tasks();
    // print_ready_tasks();
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
    // pp = pages_alloc(1, GFP_ZERO);
    // page_incref(pp);
    // print_page(pp);
    // pmem_print();
    // MAGICBP;
    // mutex_init(&m);
    task_t *tp0 = thread_start("tp0", 30, test_parent, NULL);
    task_t *tp1 = thread_start("tp1", 30, test_parent, NULL);
    task_t *tp2 = thread_start("tp2", 30, test_parent, NULL);
    task_t *tp3 = thread_start("tp0", 30, test_parent, NULL);
    task_t *tp4 = thread_start("tp1", 30, test_parent, NULL);
    task_t *tp5 = thread_start("tp2", 30, test_parent, NULL);
    thread_join(tp0);
    thread_join(tp1);
    thread_join(tp2);
    thread_join(tp3);
    thread_join(tp4);
    thread_join(tp5);
    // print_page(pp);
    // page_decref(pp);
    // print_page(pp);
    kprintf(KPL_DEBUG, "back to main\n");
    kprintf(KPL_DEBUG, "num_thread_started=%d\n", num_thread_started);
    print_all_tasks();
    print_ready_tasks();
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    for (int i = 0; i < 1000000; i++);
    kprintf(KPL_DEBUG, "later\n");
    print_all_tasks();
    print_ready_tasks();
    print_exit_tasks();
    print_sleeping_tasks();
    // vmm_print();
    // while (1) {
    //     for (int i = 0; i < 1000000; i++);
    //     for (int i = 0; i < 1000000; i++);
    //     for (int i = 0; i < 1000000; i++);
    //     for (int i = 0; i < 1000000; i++);
    //     print_all_tasks();
    //     print_ready_tasks();
    // }
    // pmem_print();
}

static void test_parent_k_get_free_pages(void *args) {
    
}

static int g = 0;
mutex_t mutex1;
static void loop482(void *a) {
    char *id = (char *)a;
    int i;
    mutex_lock(&mutex1);
    kprintf(KPL_DEBUG, "loop called with id %s\n", id);
    for (i = 0; i < 5; i++, g++) {
        kprintf(KPL_DEBUG, "%s: i=%d, g=%d\n", id, i, g);
        mutex_unlock(&mutex1);
        thread_yield();
        mutex_lock(&mutex1);
    }
    kprintf(KPL_DEBUG, "%s: i=%d, g=%d\n", id, i, g);
    mutex_unlock(&mutex1);
}

static void parent482t1(void *a) {
    intptr_t arg = (intptr_t) a;
    mutex_lock(&mutex1);
    kprintf(KPL_DEBUG, "parent called with arg %d\n", arg);
    mutex_unlock(&mutex1);
    thread_start("child", 30, loop482, "child thread");
    loop482("parent thread");
}

static void child482t10(void *a) {
    intptr_t id = (intptr_t)a;
    mutex_lock(&m);
    kprintf(KPL_DEBUG, "child %d yield\n", id);
    // mutex_unlock(&m);
    thread_yield();
    // mutex_lock(&m);
    kprintf(KPL_DEBUG, "child %d continue\n", id);
    mutex_unlock(&m);
}

static void parent482t10(void *a) {
    task_t * tasks[100];
    mutex_lock(&m);
    for (int i = 0; i < 100; i++) {
        tasks[i] = thread_start("child", 30, child482t10, i);
    }
    mutex_unlock(&m);
    for (int i = 0; i < 100; i++) {
        thread_join(tasks[i]);
    }
    kprintf(KPL_DEBUG, "parent end\n");
}

mutex_t m1, m2;

static dead1(void *args) {
    mutex_lock(&m1);
    thread_yield();
    mutex_lock(&m2);
    mutex_unlock(&m2);
    mutex_unlock(&m1);
}
static dead2(void *args) {
    mutex_lock(&m1);
    thread_yield();
    mutex_lock(&m2);
    mutex_unlock(&m1);
    mutex_unlock(&m2);
}

static int bar(int a) {
    return a + 1;
}

static void proc1() {
    int a = 1;
    a = bar(a);
    int c = a + 2;
    while (1) {
        sleep(1500);
        printf("hello proc1\n");
    }
}

static void proc2() {
    int a = 1;
    a = bar(a);
    int c = a + 2;
    // void *p = sbrk(0);
    // kprintf(KPL_DEBUG, "p=0x%X\n", p);
    void *p = brk(0);
    printf("proc2 p=0x%X\n", p);
    void *p1 = brk(p + 27 * PAGE_SIZE);
    c = *(int *)p;
    printf("proc2 c=0x%X\n", c);
    brk(p);
    // c = *(int *)p;
    printf("proc2 c=0x%X\n", c);
    printf("proc2 p1=0x%X\n", p1);
    write("hello proc2\n");
    while (1);
}


void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // test_thread();
    // pmem_print();
    // pmem_print();
    // MAGICBP;
    // test_kmalloc();
    // test_thread_kmalloc();
    // pmem_print();
    // test_page_alloc();
    // pmem_print();
    // mutex_init(&mutex1);
    // mutex_lock(&mutex1);
    // mutex_unlock(&mutex1);
    // task_t *p = thread_start("parent", 30, parent482t1, (void *)100);
    // thread_join(p);
    // mutex_init(&m1);
    // mutex_init(&m2);
    // task_t *d1 = thread_start("dead1", 30, dead1, (void *)1);
    // task_t *d2 = thread_start("dead2", 30, dead2, (void *)2);
    // thread_join(d1);
    // thread_join(d2);
    // mutex_init(&m);
    // thread_start("parent", 30, parent482t10, (void *)100);
    // vmm_print();
    // char *b0 = kmalloc(PAGE_SIZE - (48 + 16));
    // kprintf(KPL_DEBUG, "b0 = 0x%X\n", b0);
    // pmem_print();
    // char *b1 = kmalloc(PAGE_SIZE - (48 + 16) + 1);
    // kprintf(KPL_DEBUG, "b1 = 0x%X\n", b1);
    // pmem_print();
    // char *b1 = kmalloc(1320);
    // kprintf(KPL_DEBUG, "b1 = 0x%X\n", b1);
    // char *b2 = kmalloc(1300);
    // kprintf(KPL_DEBUG, "b2 = 0x%X\n", b2);

    // kfree(b0);
    // pmem_print();
    // vmm_print();
    // test_kmalloc();
    // test_kmalloc1();
    // kfree(b1);
    // kfree(b2);
    // pmem_print();
    // vmm_print();
    // process_execute(proc1, "proc1");
    // process_execute(proc2, "proc2");
    while (1) {
        thread_msleep(5 * 1000);
        kprintf(KPL_DEBUG, "kernel still works\n");
    }
    // thread_msleep(10000);
    // while(1);
}
