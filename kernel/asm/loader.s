; file loader.s
[bits 32]

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
kernel_stk_top: equ 0x01280800
usr_stk_top: equ 0x0c800000
kernel_code_sel: equ 0x10
kernel_data_sel: equ 0x18
usr_code_sel: equ 0x23
usr_data_sel: equ 0x2B
tss_sel: equ 0x30
loader:
    ; xchg bx, bx
    mov esp, kernel_stk_top
    mov ebp, esp
    ; push eax
    ; push ebx
    call kernelMain

    mov ax, tss_sel
    ltr ax

    xchg bx, bx

    push usr_data_sel  ; usr ss
    push usr_stk_top   ; usr esp
    push usr_code_sel  ; usr code
    push usr_test      ; usr func
    xchg bx, bx
    retf

    ; int 0x1F
    jmp $

global flushGDT
extern _gp  ; _gp is the gdt pointer
flushGDT:
    ; xchg bx, bx  ; magic bp
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
