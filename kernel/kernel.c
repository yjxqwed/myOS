/*   myOS kernel main code   */

#include <kprintf.h>
#include <thread/thread.h>

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

static int32_t x = 0;
static void test1(void *args) {
    char *name = (char *)args;
    while (1) {
        for (int i = 0; i < 1000000; i++);
        kprintf(KPL_DUMP, "%s: x = %d\n", name, x);
        x++;
    }
}

static void test2(void *args) {
    char *name = (char *)args;
    while (1) {
        for (int i = 0; i < 1000000; i++);
        kprintf(KPL_DUMP, "%s: x = %d\n", name, x);
        x--;
    }
}

void kernelMain() {
    kprintf(KPL_DUMP, "Hello Wolrd! --- This is myOS by Justing Yang\n");
    // kprintf(KPL_DUMP, "%d is the minimum int32\n", (int32_t)0x80000000);
    // kprintf(KPL_DUMP, "%d is the maximum int32\n", (int32_t)0x7FFFFFFF);
    // kprintf(KPL_DUMP, "%d is zero in decimal\n", 0);
    // kprintf(KPL_DUMP, "%x is zero in hexadecimal\n", 0);
    // kprintf(KPL_DUMP, "%s is a test string\n", "#abcdefg$");
    // kprintf(KPL_DUMP, "%c and %% are test characters\n", '@');

    // void *a = get_ppage(PPF_KERNEL);
    // kprintf(KPL_NOTICE, "a = %X\n", (uint32_t)a);
    // void *b = get_ppage(PPF_USER);
    // kprintf(KPL_NOTICE, "b = %X\n", (uint32_t)b);
        
    // task_t *task1 = thread_start("test1", 15, test, "abcde");
    // task_t *task2 = thread_start("test2", 5, test, "hhhh");
    task_t *task1 = thread_start("test1", 15, test1, "test++");
    task_t *task2 = thread_start("test2", 5, test2, "test--");
    // for (int i = 0; ; i++) {
    //     if (i % 1000000 == 0) {
    //         // print_ready_tasks();
    //         kprintf(KPL_DEBUG, " main ");
    //         i = 0;
    //     }
    // }
    // thread_join(task1);
    // thread_join(task2);
    // kprintf(KPL_DEBUG, " main ");
    while (1) {
        // kprintf(KPL_DEBUG, " main ");
    }
    while (1);
}
