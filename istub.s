; file for the interrupt service routines
[bits 32]

%macro isr_no_err_code 1
global isr%1
isr%1:
    push byte 0  ; dummy error code
    push byte %1 ; int number
    jmp isr_common_stub
%endmacro

[section .text]

; global isr0
isr_no_err_code 0  ; 0: Divide By Zero Exception


; The C interrupt handler
extern interrupt_handler

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level fault handler,
; and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x18   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us the stack
    push eax
    mov eax, interrupt_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!