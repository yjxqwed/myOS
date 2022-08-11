#ifndef __X86_H__
#define __X86_H__

/**************************************
 * 
 * This file contains the x86 platform
 *  related macros.
 * 
***************************************/

#include <common/types.h>

// =================== PAGING/MMU =====================

typedef uint32_t PageDirectoryEntry;
typedef PageDirectoryEntry pde_t;
typedef uint32_t PageTableEntry;
typedef PageTableEntry pte_t;

// page size = 4KiB
#define PAGE_SIZE 0x1000

#define __ALLIGNED(align) __attribute__((aligned(align)))

// page alligned
#define __PAGE_ALLIGNED __attribute__((aligned(PAGE_SIZE)))

// page directory shift -- the high 10 bits is page directory idx
#define PD_IDX_SHIFT 22
// page table shift -- the mid 10 bits is page table idx
#define PT_IDX_SHIFT 12

// x86 use 10-bit index for pde and pte
#define PD_IDX_MASK (0x3ff << PD_IDX_SHIFT)
#define PT_IDX_MASK (0x3ff << PT_IDX_SHIFT)

#define PG_START_ADDRESS_MASK 0xfffff000
#define PG_OFFSET_MASK 0xfff

#define __pg_start_addr(x) ((uintptr_t)(x) & PG_START_ADDRESS_MASK)

#define __pde_idx(x) ((uintptr_t)(x) >> PD_IDX_SHIFT)
#define __pte_idx(x) (((uintptr_t)(x) & PT_IDX_MASK) >> PT_IDX_SHIFT)

#define __pg_entry(page_pa, attr) ((pte_t)((uintptr_t)(page_pa) | (attr)))

#define __page_number(x) (uint32_t)(((uintptr_t)(x)) / PAGE_SIZE)

#define __page_aligned(x) (((uintptr_t)(x) & PG_OFFSET_MASK) == 0 ? True : False)

// ========== Page Directory ==========

// ignored
#define PDE_GLOBAL        0x100
// 0 = 4KiB, 1 = 4MiB (PSE needed)
// should be 0
#define PDE_PAGE_SIZE     0x080
// accessed (set by CPU)
#define PDE_ACCESSED      0x020
// set to disable cache
#define PDE_CACHE_DISABLE 0x010
// set to enable write-through caching
// clear to enable write-back caching
#define PDE_WRITE_THRU    0x008
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PDE_USER          0x004
// set to make it writable; clear to make it read-only
#define PDE_WRITABLE      0x002
#define PDE_READABLE      0x000
// set = in mem; clear = swapped-out
#define PDE_PRESENT       0x001

// void set_pde_attr(pde_t *pde, int attr, int val);

// ========== Page Table ==========

// set to prevent the TLB from updating the
// address in its cache if cr3 is reset
// (page global enable bit in cr4 must also be set)
#define PTE_GLOBAL        0x100
// unused (too complex)
#define PTE_PAGE_TABLE_ATTR_IDX 0x080
// set = this page is written to (set by cpu)
#define PTE_DIRTY         0x040
// set = accessed (set by cpu)
#define PTE_ACCESSED      0x020
// set to disable cache
#define PTE_CACHE_DISABLE 0x010
// set to enable write-through caching
// clear to enable write-back caching
#define PTE_WRITE_THRU    0x008
// set to make it accessible to all
// clear to make it only accessible to superuser
#define PTE_USER          0x004
// set to make it writable; clear to make it read-only
#define PTE_WRITABLE      0x002
#define PTE_READABLE      0x000
// set = in mem; clear = swapped-out
#define PTE_PRESENT       0x001

// number of pdes per page directory
#define NRPDE 1024
// number of ptes per page table
#define NRPTE 1024

// amount of memory controlled by a pg_tab (4 MiB)
#define MEM_SIZE_PER_PGTABLE (NRPTE * PAGE_SIZE)

// ====================== Register ============================

#define CR0_PG 0x80000000

// ================== Inline Assembly =========================

#define __attr_always_inline __attribute__((always_inline))

// read a byte from io port
static inline uint8_t inportb(uint16_t port) __attr_always_inline;
// write a byte to io port
static inline void outportb(uint16_t port, uint8_t val) __attr_always_inline;
/**
 * @brief read from port and store to buf
 * @param port port number
 * @param buf data buffer
 * @param word_cnt number of words (2 bytes) to read
 */
static inline void inportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) __attr_always_inline;

/**
 * @brief write data to port from buf
 * @param port port number
 * @param buf data buffer
 * @param word_cnt number of words (2 bytes) to write
 */
static inline void outportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) __attr_always_inline;

// load gdt
static inline void lgdt(void *gp) __attr_always_inline;

// load idt
static inline void lidt(void *ip) __attr_always_inline;

// load cr0
static inline void lcr0(uint32_t x) __attr_always_inline;
// store cr0
static inline uint32_t scr0() __attr_always_inline;

// load cr3
static inline void lcr3(uint32_t x) __attr_always_inline;
// store cr3
static inline uint32_t scr3() __attr_always_inline;

// invlpg
static inline void invlpg(void *va) __attr_always_inline;

// load task register (tss)
static inline void ltr(uint16_t selector_tss) __attr_always_inline;

static inline void hlt() __attr_always_inline;

#define __asm_volatile __asm__ volatile

static inline uint8_t inportb(uint16_t port) {
    uint8_t val;
    __asm_volatile(
        "inb %0, %1\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

static inline void outportb(uint16_t port, uint8_t val) {
    __asm_volatile(
        "outb %1, %0\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        :             // output
        : "a"(val), "Nd"(port)  // input
        :             // clobbered regs
    );
}

static inline void inportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) {
    __asm_volatile(
        "cld;"
        "rep insw;"
        : "+D"(buf), "+c"(word_cnt)
        : "Nd"(port)
        : "memory", "cc"
    );
}

static inline void outportsw(
    uint16_t port, void *buf, uint32_t word_cnt
) {
    __asm_volatile(
        "cld;"
        "rep outsw;"
        : "+S"(buf), "+c"(word_cnt)
        : "Nd"(port)
        : "cc"
    );
}

static inline uint32_t inportl(uint16_t port) {
    uint32_t val;
    __asm_volatile (
        "ind %0, %1\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        : "=a"(val)   // output
        : "Nd"(port)  // input
        :             // clobbered regs
    );
    return val;
}

static inline void outportl(uint16_t port, uint32_t val) {
    __asm_volatile (
        "outd %1, %0\n\t"
        "nop\n\t"
        "nop"         // introduce some delay
        :             // output
        : "a"(val), "Nd"(port)  // input
        :             // clobbered regs
    );
}

static inline void lgdt(void *gp) {
    __asm_volatile (
        "lgdt [%0]"
        :
        : "r"(gp)  // input
        :
    );
}

static inline void lidt(void *ip) {
    __asm_volatile (
        "lidt [%0]"
        :
        : "r"(ip)  // input
        :
    );
}

static inline void lcr0(uint32_t x) {
    __asm_volatile (
        "mov cr0, %0"
        :
        : "r"(x)
        :
    );
}

static inline uint32_t scr0() {
    uint32_t cr0;
    __asm_volatile (
        "mov %0, cr0"
        : "=r"(cr0)
        :
        :
    );
    return cr0;
}

static inline void lcr3(uint32_t x) {
    __asm_volatile (
        "mov cr3, %0"
        :
        : "r"(x)
        :
    );
}

static inline uint32_t scr3() {
    uint32_t cr3;
    __asm_volatile (
        "mov %0, cr3"
        : "=r"(cr3)
        :
        :
    );
    return cr3;
}

static inline uint32_t scr2() {
    uint32_t cr2;
    __asm_volatile (
        "mov %0, cr2"
        : "=r"(cr2)
        :
        :
    );
    return cr2;
}

static inline void invlpg(void *va) {
    __asm_volatile (
        "invlpg [%0]"
        :
        : "r"(va)
        : "memory"
    );
}

static inline void ltr(uint16_t selector_tss) {
    __asm_volatile (
        "ltr %0"
        :
        : "r"(selector_tss)
        :
    );
}

static inline void hlt() {
    __asm_volatile (
        "hlt"
        :
        :
        : "memory"
    );
}

// IF is bit 9 of eflags
#define EFLAGS_IF_MASK 0x00000200

#define EFLAGS_MSB(x)  ((x) << 1)
#define EFLAGS_IF(x)   ((x) << 9)
#define EFLAGS_IOPL(x) ((x) << 12)

// for enable/disable interrupts (by setting/clearing IF)

typedef enum INT_STATUS {
    INTERRUPT_OFF,
    INTERRUPT_ON
} INT_STATUS;

// get current interrupt status
INT_STATUS get_int_status();

// set interrupt status
void set_int_status(INT_STATUS status);

// enable interrupt and return the previous status
INT_STATUS enable_int();

// disable interrupt and return the previous status
INT_STATUS disable_int();

/**
 * 8259A chip ports. More info: https://wiki.osdev.org/8259_PIC
 * x86 Programable Interrupt Controller
 */
#define PIC_M_CTL     0x20
#define PIC_M_CTLMASK 0x21
#define PIC_S_CTL     0xA0
#define PIC_S_CTLMASK 0xA1

// end of interrupt
#define PIC_EOI       0x20

/**
 * More info: https://wiki.osdev.org/Interrupt
 */
#define IRQ_PIT 0x0
#define IRQ_KB  0x1
#define IRQ_CASCADE 0x2
#define IRQ_COM2 0x3
#define IRQ_COM1 0x4
#define IRQ_LPT2 0x5
#define IRQ_FLOPPY 0x6
#define IRQ_LPT1 0x7
#define IRQ_CLOCK 0x8
#define IRQ_PS2 0xC
#define IRQ_PATA 0xE
#define IRQ_SATA 0xF


void disable_pit();
void enable_pit();

/**
 * @brief disable one type of ints
 */
void IRQ_set_mask(unsigned char IRQline);

/**
 * @brief enable one type of ints
 */
void IRQ_clear_mask(unsigned char IRQline);

#endif
