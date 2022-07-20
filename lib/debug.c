#include <common/debug.h>
#include <string.h>
#include <kprintf.h>
#include <arch/x86.h>
#include <thread/thread.h>

void printISRParam(const isrp_t* p) {
    task_t *t = get_current_thread();
    console_kprintf(
        KPL_PANIC,
        "\n[%s, %d]"
        "{eip=%x; errco=%x; "
        "edi=%X, esi=%X, ebx=%X, edx=%X, ecx=%X, eax=%X}\n",
        t->task_name, t->task_id,
        p->eip, p->err_code,
        p->edi, p->esi, p->ebx, p->edx, p->ecx, p->eax
    );
}

void panic_spin(
    const char* filename, int line,
    const char* funcname, const char* cause
) {
    disable_int();
    console_kprintf(KPL_DUMP, "\n");
    console_kprintf(KPL_PANIC, "!!!!! error !!!!!\n");
    console_kprintf(KPL_PANIC, "file: %s\n", filename);
    console_kprintf(KPL_PANIC, "line: %d\n", line);
    console_kprintf(KPL_PANIC, "function: %s\n", funcname);
    console_kprintf(KPL_PANIC, "cause: %s\n", cause);
    hlt();
}
