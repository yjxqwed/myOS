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

// ================== Assembly =========================

#define __asm_volatile __asm__ volatile

uint8_t inportb(uint16_t port);
void outportb(uint16_t port, uint8_t val);
uint32_t inportd(uint16_t port);
void outportd(uint16_t port, uint32_t val);


void inportsw(uint16_t port, void *buf, uint32_t word_cnt);
void outportsw(uint16_t port, void *buf, uint32_t word_cnt);
void inportsd(uint16_t port, void *buf, uint32_t dword_cnt);
void outportsd(uint16_t port, void *buf, uint32_t dword_cnt);


// ====================== Register ============================

#define CR0_PG 0x80000000

// load gdt
void lgdt(void *gp);

// load idt
void lidt(void *ip);

// load cr0
void lcr0(uint32_t x);
// store cr0
uint32_t scr0();

// load cr3
void lcr3(uint32_t x);
// store cr3
uint32_t scr3();

// invlpg
void invlpg(void *va);

// load task register (tss)
void ltr(uint16_t selector_tss);

void hlt();

// ================ INTERRUPT ================

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
