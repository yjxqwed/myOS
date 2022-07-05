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


static void start_process(void *filename) {
    void *func = filename;
    uint32_t ss = SELECTOR_USTK;
    uint32_t esp = USER_STACK_BOTTOM - 0x10;
    uint32_t eflags = EFLAGS_MSB(1) | EFLAGS_IF(1);
    uint32_t cs = SELECTOR_UCODE;
    uint32_t eip = (uint32_t)func;
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

task_t *process_execute(char *filename, char *name, int tty_no) {
    task_t *t = task_create(name, 31, start_process, filename);
    // cwd is root(/) by default for now
    // t->cwd_inode_no = 0;
    int rollback = 0;
    if (t == NULL) {
        goto rb;
    }
    t->is_user_process = True;
    t->vmm = (vmm_t *)kmalloc(sizeof(vmm_t));
    if (t->vmm == NULL || init_vmm_struct(t->vmm) != 0) {
        rollback = 1;
        goto rb;
    }
    init_vmm_struct(t->vmm);
    t->fd_table = (int *)kmalloc(NR_OPEN * sizeof(int));
    for (int i = 0; i < NR_OPEN; i++) {
        t->fd_table[i] = i < 3 ? i : -1;
    }
    if (t->fd_table == NULL) {
        rollback = 2;
        goto rb;
    }
    INT_STATUS old_status = disable_int();
    ASSERT(t->status == TASK_READY);
    task_push_back_ready(t);
    task_push_back_all(t);
    task_assign_tty(t, tty_no);
    set_int_status(old_status);
    return t;

rb:
    switch (rollback) {
        case 2:
            destroy_vmm_struct(t->vmm);
            kfree(t->vmm);
        case 1:
            k_free_pages(t, 1);
        default:
            return NULL;
    }
}

// /* 将文件描述符fd指向的文件中, 偏移为offset, 大小为filesz的段加载到虚拟地址为vaddr的内存 */
// static bool segment_load(int32_t fd, uint32_t offset, uint32_t filesz, uint32_t vaddr) {
//     uint32_t vaddr_first_page = vaddr & 0xfffff000;    // vaddr地址所在的页框
//     uint32_t size_in_first_page = PG_SIZE - (vaddr & 0x00000fff);     // 加载到内存后,文件在第一个页框中占用的字节大小
//     uint32_t occupy_pages = 0;
//     /* 若一个页框容不下该段 */
//     if (filesz > size_in_first_page) {
//         uint32_t left_size = filesz - size_in_first_page;
//         occupy_pages = DIV_ROUND_UP(left_size, PG_SIZE) + 1;	     // 1是指vaddr_first_page
//     } else {
//         occupy_pages = 1;
//     }

//     /* 为进程分配内存 */
//     uint32_t page_idx = 0;
//     uint32_t vaddr_page = vaddr_first_page;
//     while (page_idx < occupy_pages) {
//         uint32_t* pde = pde_ptr(vaddr_page);
//         uint32_t* pte = pte_ptr(vaddr_page);

//         /* 如果pde不存在,或者pte不存在就分配内存.
//         * pde的判断要在pte之前,否则pde若不存在会导致
//         * 判断pte时缺页异常 */
//         if (!(*pde & 0x00000001) || !(*pte & 0x00000001)) {
//         if (get_a_page(PF_USER, vaddr_page) == NULL) {
//             return false;
//         }
//         } // 如果原进程的页表已经分配了,利用现有的物理页,直接覆盖进程体
//         vaddr_page += PG_SIZE;
//         page_idx++;
//     }
//     sys_lseek(fd, offset, SEEK_SET);
//     sys_read(fd, (void*)vaddr, filesz);
//     return true;
// }

// static int segment_load(int fd, const elf32_phdr_t *phdr) {
    
// }

// /* 从文件系统上加载用户程序pathname,成功则返回程序的起始地址,否则返回-1 */
// static int load(const char *pathname) {
//     int ret = -1;
//     struct Elf32_Ehdr elf_header;
//     struct Elf32_Phdr prog_header;
//     int elf32_hdr_sz = sizeof(struct Elf32_Ehdr);

//     memset(&elf_header, 0, elf32_hdr_sz);

//     int fd = sys_open(pathname, O_RDONLY);
//     if (fd == -1) {
//         return -1;
//     }

//     if (sys_read(fd, &elf_header, elf32_hdr_sz) != elf32_hdr_sz) {
//         ret = -1;
//         goto done;
//     }

//     /* verify elf header */
//     if (
//         memcmp(elf_header.e_ident, "\177ELF\1\1\1\0\0\0\0\0\0\0\0\0", 16) != 0 ||  // elf32 idents
//         elf_header.e_type != 2 ||  // ET_EXEC (executable file)
//         elf_header.e_machine != 3 ||  // x86
//         elf_header.e_version != 1 ||  // current version (default value)
//         elf_header.e_phnum > 1024 ||
//         elf_header.e_phentsize != elf32_hdr_sz
//     ) {
//         ret = -1;
//         goto done;
//     }

//     Elf32_Off prog_header_offset = elf_header.e_phoff; 
//     Elf32_Half prog_header_size = elf_header.e_phentsize;

//     /* 遍历所有程序头 */
//     uint32_t prog_idx = 0;
//     while (prog_idx < elf_header.e_phnum) {
//         memset(&prog_header, 0, prog_header_size);

//         /* 将文件的指针定位到程序头 */
//         sys_lseek(fd, prog_header_offset, SEEK_SET);

//         /* 只获取程序头 */
//         if (sys_read(fd, &prog_header, prog_header_size) != prog_header_size) {
//             ret = -1;
//             goto done;
//         }

//         /* 如果是可加载段就调用segment_load加载到内存 */
//         if (PT_LOAD == prog_header.p_type) {
//             if (!segment_load(fd, prog_header.p_offset, prog_header.p_filesz, prog_header.p_vaddr)) {
//                 ret = -1;
//                 goto done;
//             }
//         }

//         /* 更新下一个程序头的偏移 */
//         prog_header_offset += elf_header.e_phentsize;
//         prog_idx++;
//     }
//     ret = elf_header.e_entry;
// done:
//     sys_close(fd);
//     return ret;
// }