; file for the interrupt stubs
; one of these stubs will be called when
; an interrupt is raised
[bits 32]

%include "sys/asm/inc.s"

; interrupt gate will automatically clear the IF bit (cli)

%macro isr_no_err_code 1
global isr%1
isr%1:
    push byte 0  ; dummy error code
    push byte %1 ; int number
    jmp isr_common_stub
%endmacro

%macro isr_err_code 1
global isr%1
isr%1:
    push byte %1 ; int number
    jmp isr_common_stub
%endmacro;

[section .text]

; cpu exception handlers
isr_no_err_code 0   ; 0:  Divide By Zero Exception
isr_no_err_code 1   ; 1:  Debug Execption
isr_no_err_code 2   ; 2:  Non Maskable Interrupt Execption
isr_no_err_code 3   ; 3:  Breakpoint Execption
isr_no_err_code 4   ; 4:  Into Detected Overflow Exception
isr_no_err_code 5   ; 5:  Out of Bounds Exception
isr_no_err_code 6   ; 6:  Invalid Opcode Exception
isr_no_err_code 7   ; 7:  No Coprocessor Exception
isr_err_code 8      ; 8:  Double Fault Exception
isr_no_err_code 9   ; 9:  Coprocessor Segment Overrun Exception
isr_err_code 10     ; 10: Bad TSS Exception
isr_err_code 11     ; 11: Segment Not Present Exception
isr_err_code 12     ; 12: Stack Fault Exception
isr_err_code 13     ; 13: General Protection Fault Exception
isr_err_code 14     ; 14: Page Fault Exception
isr_no_err_code 15  ; 15: Unknown Interrupt Exception
isr_no_err_code 16  ; 16: Coprocessor Fault Exception
isr_no_err_code 17  ; 17: Alignment Check Exception (486+)
isr_no_err_code 18  ; 18: Machine Check Exception (Pentium/586+)
isr_no_err_code 19  ; 19: Reserved
isr_no_err_code 20  ; 20: Reserved
isr_no_err_code 21  ; 21: Reserved
isr_no_err_code 22  ; 22: Reserved
isr_no_err_code 23  ; 23: Reserved
isr_no_err_code 24  ; 24: Reserved
isr_no_err_code 25  ; 25: Reserved
isr_no_err_code 26  ; 26: Reserved
isr_no_err_code 27  ; 27: Reserved
isr_no_err_code 28  ; 28: Reserved
isr_no_err_code 29  ; 29: Reserved
isr_no_err_code 30  ; 30: Reserved
isr_no_err_code 31  ; 31: Reserved

; interrupt requests handlers
; isr 32-47 = irq 0 - 15
isr_no_err_code 32  ; 32: IRQ0
isr_no_err_code 33  ; 33: IRQ1
isr_no_err_code 34  ; 34: IRQ2
isr_no_err_code 35  ; 35: IRQ3
isr_no_err_code 36  ; 36: IRQ4
isr_no_err_code 37  ; 37: IRQ5
isr_no_err_code 38  ; 38: IRQ6
isr_no_err_code 39  ; 39: IRQ7
isr_no_err_code 40  ; 40: IRQ8
isr_no_err_code 41  ; 41: IRQ9
isr_no_err_code 42  ; 42: IRQ10
isr_no_err_code 43  ; 43: IRQ11
isr_no_err_code 44  ; 44: IRQ12
isr_no_err_code 45  ; 45: IRQ13
isr_no_err_code 46  ; 46: IRQ14
isr_no_err_code 47  ; 47: IRQ15




; The C interrupt handler
extern interrupt_handler
; extern _dss

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level interrupt handler,
; and finally restores the stack frame.
isr_common_stub:
    pushad
    push ds
    push es
    push fs
    push gs
    mov ax, kernel_data_sel   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp  ; push the param
    call interrupt_handler
    pop eax  ; pop the param
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
                   ; Although the CPU pushes error code automatically, it doesn't pop it

    iretd          ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

; push esp pushes the original value of esp
;
; |    |                push esp  |0x0C| <- esp = 0x08  high addr
; |xxxx| <- esp = 0x0C    ===>    |xxxx|
; |yyyy|                          |yyyy|                low addr