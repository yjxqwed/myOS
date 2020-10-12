#ifndef __PROC_H__
#define __PROC_H__
// multitasking related

#include <common/types.h>
#include <sys/gdt.h>  // for segment descriptor

// struct for saving the execution state of a process
typedef struct Registers {
    // pushed by us
    uint32_t gs, fs, es, ds;
    // pushed by us, 'pushad' (eax first pushed, edi last pushed)
    uint32_t edi, esi, ebp, kernal_esp, ebx, edx, ecx, eax;
    // currently UNKNOWN
    uint32_t ret_addr;
    // pushed by the CPU automatically during an interrupt
    uint32_t eip, cs, eflags, user_esp, ss;
} stack_frame_t;


// each process has its own LDT, which consists of 
// PROC_LDT_SIZE descriptors
#define PROC_LDT_SIZE 2

// each process has a name no longer then 
// PROC_NAME_LENGTH - 1 characters (null terminated)
#define PROC_NAME_LENGTH 16

// struct for each process
typedef struct Process {
    // registers for state saving
    stack_frame_t regs;
    // LDT selector indicating a descriptor to 
    // the LDT of this process in the GDT.
    uint16_t ldt_sel;
    // LDT of this process
    seg_des_t ldt[PROC_LDT_SIZE];
    // PID
    uint16_t pid;
    // parent's PID
    uint16_t parent_pid;
    // process name
    char pname[PROC_NAME_LENGTH];
} proc_t;


// maximum number of processes allowed
#define NUM_PROCESS_LIMIT 1

#endif