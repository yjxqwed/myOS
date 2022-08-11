/*   myOS kernel main code   */

#include <lib/kprintf.h>
#include <thread/process.h>
#include <arch/x86.h>

static void print_myOS(void) {
    kprintf(KPL_DUMP, "\n");
    kprintf(
        KPL_DUMP,
        ">>=====================================================<<\n"
        "||                            #######     ######       ||\n"
        "||  WELCOME TO               ##     ##   ##     #      ||\n"
        "||                           ##     ##   ##            ||\n"
        "||     ##     ##  ##     ##  ##     ##    ######       ||\n"
        "||     #### ####   ##   ##   ##     ##         ##      ||\n"
        "||     ## ### ##    ## ##    ##     ##   #     ##      ||\n"
        "||     ##     ##     ###      #######     #####        ||\n"
        "||                   ##                                ||\n"
        "||                  ##              by Jiaxing Yang    ||\n"
        "||                                       ver: 0.0.1    ||\n"
        ">>=====================================================<<\n"
    );
}

extern void kernel_test_simplefs();

void kernelMain() {
    // kprintf(KPL_DUMP, "\nHello Wolrd! --- This is myOS by Justing Yang\n");
    print_myOS();
    process_execute("login", "login", 0, 0, NULL);
    process_execute("login", "login", 1, 0, NULL);
    while (1) {
        INT_STATUS old_status = disable_int();
        ASSERT(old_status == INTERRUPT_ON);
        thread_block_self(TASK_BLOCKED);
        set_int_status(old_status);
        hlt();
    }
}
