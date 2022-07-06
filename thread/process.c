#include <thread/process.h>
#include <arch/x86.h>
#include <mm/pmem.h>
#include <mm/kvmm.h>
#include <lib/kprintf.h>
#include <common/debug.h>
#include <sys/gdt.h>
#include <myos.h>
#include <common/types.h>
#include <fs/simplefs/simplefs.h>

typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

typedef struct Elf32_Ehdr {
   unsigned char e_ident[16];
   Elf32_Half    e_type;
   Elf32_Half    e_machine;
   Elf32_Word    e_version;
   Elf32_Addr    e_entry;
   Elf32_Off     e_phoff;
   Elf32_Off     e_shoff;
   Elf32_Word    e_flags;
   Elf32_Half    e_ehsize;
   Elf32_Half    e_phentsize;
   Elf32_Half    e_phnum;
   Elf32_Half    e_shentsize;
   Elf32_Half    e_shnum;
   Elf32_Half    e_shstrndx;
} elf32_ehdr_t;

typedef struct Elf32_Phdr {
   Elf32_Word p_type;
   Elf32_Off  p_offset;
   Elf32_Addr p_vaddr;
   Elf32_Addr p_paddr;
   Elf32_Word p_filesz;
   Elf32_Word p_memsz;
   Elf32_Word p_flags;
   Elf32_Word p_align;
} elf32_phdr_t;

enum segment_type {
   PT_NULL,            // 忽略
   PT_LOAD,            // 可加载程序段
   PT_DYNAMIC,         // 动态加载信息 
   PT_INTERP,          // 动态加载器名称
   PT_NOTE,            // 一些辅助信息
   PT_SHLIB,           // 保留
   PT_PHDR             // 程序头表
};

static int read_elf32_ehdr(int fd, elf32_ehdr_t *ehdr) {
    ASSERT(fd != -1 && ehdr != NULL);
    int elf32_ehdr_sz = sizeof(elf32_ehdr_t);
    memset(ehdr, 0, elf32_ehdr_sz);
    int ret = -1;
    if (simplefs_file_read(fd, ehdr, elf32_ehdr_sz) != elf32_ehdr_sz) {
        // failed to read
    } else if (
        memcmp(ehdr->e_ident, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0", 16) != 0 ||  // elf32 idents
        ehdr->e_type != 2 ||  // ET_EXEC (executable file)
        ehdr->e_machine != 3 ||  // x86
        ehdr->e_version != 1 ||  // current version (default value)
        ehdr->e_phnum > 1024 ||
        ehdr->e_phentsize != sizeof(elf32_phdr_t)
    ) {
        // failed to verify ehdr
    } else {
        // good to go
        ret = 0;
    }
    return ret;
}

static int segment_load(int fd, pde_t *new_pd, const elf32_phdr_t *phdr) {
    ASSERT(phdr->p_type == PT_LOAD);

    uint32_t vaddr_first_page = __pg_start_addr(phdr->p_vaddr);
    uint32_t num_pages = ROUND_UP_DIV(phdr->p_memsz + phdr->p_vaddr - vaddr_first_page, PAGE_SIZE);

    vmm_map_pages(new_pd, vaddr_first_page, num_pages, PTE_USER | PTE_WRITABLE);
    task_t *ct = get_current_thread();
    pde_t *curr_pd = NULL;

    if (ct->vmm != NULL) {
        curr_pd = ct->vmm->pgdir;
    }
    load_page_dir(new_pd);
    off_t off = simplefs_file_lseek(fd, phdr->p_offset, SEEK_SET);
    ASSERT(off == phdr->p_offset);
    kprintf(KPL_DEBUG, "off=0x%x, vfp=0x%x, fsz=0x%x", off, vaddr_first_page, phdr->p_filesz);
    simplefs_file_read(fd, vaddr_first_page, phdr->p_filesz);
    for (int i = 0; i < 10; i++) {
        kprintf(KPL_DEBUG, "{0x%X}", *((uint32_t *)vaddr_first_page + i));
    }
    load_page_dir(curr_pd);
    return 0;
}

static int load(int fd, const elf32_ehdr_t *ehdr, pde_t *pd) {
    // INT_STATUS old_status = disable_int();

    int ret = -1;
    elf32_phdr_t phdr;

    Elf32_Off prog_header_offset = ehdr->e_phoff; 
    Elf32_Half prog_header_size = ehdr->e_phentsize;

    /* 遍历所有程序头 */
    uint32_t prog_idx = 0;
    while (prog_idx < ehdr->e_phnum) {
        memset(&phdr, 0, prog_header_size);

        /* 将文件的指针定位到程序头 */
        off_t off = simplefs_file_lseek(fd, prog_header_offset, SEEK_SET);
        ASSERT(off == prog_header_offset);
        /* 只获取程序头 */
        int nbrd = simplefs_file_read(fd, &phdr, prog_header_size);
        if (nbrd != prog_header_size) {
            goto __load_done__;
        }

        /* 如果是可加载段就调用segment_load加载到内存 */
        if (PT_LOAD == phdr.p_type) {
            if (segment_load(fd, pd, &phdr) != 0) {
                goto __load_done__;
            }
        }

        /* 更新下一个程序头的偏移 */
        prog_header_offset += ehdr->e_phentsize;
        prog_idx++;
    }

    ret = 0;

__load_done__:
    // set_int_status(old_status);
    return ret;
}


static void start_process(void *entry) {
    uint32_t ss = SELECTOR_USTK;
    uint32_t esp = USER_STACK_BOTTOM - 0x10;
    uint32_t eflags = EFLAGS_MSB(1) | EFLAGS_IF(1);
    uint32_t cs = SELECTOR_UCODE;
    uint32_t eip = (uint32_t)entry;
    __asm_volatile(
        "push %0\n\t"
        "push %1\n\t"
        "push %2\n\t"
        "push %3\n\t"
        "push %4\n\t"
        "mov ds, %5\n\t"
        "mov es, %5\n\t"
        "mov fs, %5\n\t"
        "mov gs, %5\n\t"
        "mov ebp, %1\n\t"
        "iretd"
        :
        : "r"(ss),
          "r"(esp),
          "r"(eflags),
          "r"(cs),
          "r"(eip),
          "r"(SELECTOR_UDATA)
        :
    );
}

static void process_destroy(task_t *task) {
    if (task == NULL) {
        return;
    }
    destroy_vmm_struct(task->vmm);
    kfree(task->vmm);
    kfree(task->fd_table);
    task_destroy(task);
}

task_t *process_execute(const char *filename, const char *name, int tty_no) {
    // open target file
    int fd = simplefs_file_open(filename, 0);
    if (fd == -1) {
        goto __process_execute_fail__;
    }

    // read elf32 header
    elf32_ehdr_t ehdr;
    if (read_elf32_ehdr(fd, &ehdr) != 0) {
        goto __process_execute_fail__;
    }

    // create a task object
    task_t *t = task_create(name, 31, start_process, ehdr.e_entry);
    if (t == NULL) {
        goto __process_execute_fail__;
    }

    // create a vmm object
    t->vmm = (vmm_t *)kmalloc(sizeof(vmm_t));
    if (t->vmm == NULL || init_vmm_struct(t->vmm) != 0) {
        goto __process_execute_fail__;
    }

    // create the fd table
    t->fd_table = (int *)kmalloc(NR_OPEN * sizeof(int));
    if (t->fd_table == NULL) {
        goto __process_execute_fail__;
    }
    for (int i = 0; i < NR_OPEN; i++) {
        t->fd_table[i] = i < 3 ? i : -1;
    }

    // load segments
    if (load(fd, &ehdr, t->vmm->pgdir) != 0) {
        goto __process_execute_fail__;
    }

    // assign tty
    task_assign_tty(t, tty_no);
    t->is_user_process = True;

    // push the task to lists
    INT_STATUS old_status = disable_int();
    ASSERT(t->status == TASK_READY);
    task_push_back_ready(t);
    task_push_back_all(t);
    set_int_status(old_status);
    simplefs_file_close(fd);
    return t;

__process_execute_fail__:
    simplefs_file_close(fd);
    process_destroy(t);
    return NULL;
}

pid_t sys_getpid() {
    return get_current_thread()->task_id;
}

pid_t sys_getppid() {
    return get_current_thread()->parent_id;
}