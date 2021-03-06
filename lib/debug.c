#include <common/debug.h>
#include <string.h>
#include <kprintf.h>
#include <arch/x86.h>
#include <thread/thread.h>

void printISRParam(const isrp_t* p) {
    kprintf(
        KPL_PANIC, 
        "\n{eip=%x; errco=%x; "
        "edi=%X, esi=%X, ebx=%X, edx=%X, ecx=%X, eax=%X}\n",
        p->eip, p->err_code,
        p->edi, p->esi, p->ebx, p->edx, p->ecx, p->eax, p->eax
    );
}

void panic_spin(
    const char* filename, int line,
    const char* funcname, const char* cause
) {
    disable_int();
    kprintf(KPL_DUMP, "\n");
    kprintf(KPL_PANIC, "!!!!! error !!!!!\n");
    kprintf(KPL_PANIC, "file: %s\n", filename);
    kprintf(KPL_PANIC, "line: %d\n", line);
    kprintf(KPL_PANIC, "function: %s\n", funcname);
    kprintf(KPL_PANIC, "cause: %s\n", cause);
    // print_all_tasks();
    hlt();
    // while (1);
}
