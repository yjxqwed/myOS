; file entry.s: start point of kernel
[bits 32]

%include "sys/asm/inc.s"

[section .multiboot]
    MAGIC_NUM equ 0x1badb002
    FLAG      equ (1 << 0 | 1 << 1)
    CHECK_SUM equ -(MAGIC_NUM + FLAG)

    dd MAGIC_NUM
    dd FLAG
    dd CHECK_SUM

extern entry_setup
extern ksetup

[section .text]
global entry
entry:
    mov esp, boot_stk - KERNEL_BASE
    mov ebp, esp

    push eax
    push ebx
    call entry_setup
    add esp, 8
    ; jmp to the higher half
    lea eax, [paging_relocation]
    jmp eax
paging_relocation:
    add esp, KERNEL_BASE
    add ebp, KERNEL_BASE
    call ksetup

    ; setup done, jmp to the kernel
    jmp kernel_entry


global flushGDT
flushGDT:
    mov ax, kernel_data_sel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp kernel_code_sel:flush2
flush2:
    ret


extern kernelMain
; start of the kernel
kernel_entry:
    call kernelMain
    jmp $

[section .data]
times 1024 dw 0
boot_stk:
