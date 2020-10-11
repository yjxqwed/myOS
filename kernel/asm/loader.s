; file loader.s
[bits 32]

%include "sys/asm/inc.s"

[section .multiboot]
    MAGIC_NUM equ 0x1badb002
    FLAG      equ (1 << 0 | 1 << 1)
    CHECK_SUM equ -(MAGIC_NUM + FLAG)   

    dd MAGIC_NUM
    dd FLAG
    dd CHECK_SUM

extern kernelMain
extern usr_test
[section .text]
global loader

loader:
    mov esp, kernel_stk_top
    mov ebp, esp
    push eax
    push ebx
    call kernelMain

    ; xchg bx, bx
    ; magic_bp

    ; go to ring 3
    mov ax, tss_sel
    ltr ax
    push usr_data_sel  ; usr ss
    push usr_stk_top   ; usr esp
    push usr_code_sel  ; usr code
    push usr_test      ; usr func
    retf

    jmp $

global flushGDT
extern _gp  ; _gp is the gdt pointer
flushGDT:
    lgdt [_gp]
    mov ax, kernel_data_sel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp kernel_code_sel:flush2
flush2:
    ret

global flushIDT
extern _ip  ; the idt pointer
flushIDT:
    lidt [_ip]
    ret

[section .data]
one: db 0x01
