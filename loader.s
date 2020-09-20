; file loader.s
[bits 32]

[section .multiboot]
    MAGIC_NUM equ 0x1badb002
    FLAG equ (1 << 0 | 1 << 1)
    CHECK_SUM equ -(MAGIC_NUM + FLAG)   

    dd MAGIC_NUM
    dd FLAG
    dd CHECK_SUM

extern kernelMain
[section .text]
global loader
loader:
    xchg bx, bx  ; magic bp
    mov esp, kernel_stk
    mov ebp, esp
    push eax
    push ebx
    call kernelMain
    loop:
        jmp loop

global flushGDT
extern _gp, _dss, _css
flushGDT:
    xchg bx, bx  ; magic bp
    lgdt [_gp]
    mov ax, [_dss]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ax, [_css]
    jmp 0x10:flush2
flush2:
    ret


[section .bss]
    resb 2*1024*1024  ; 2MiB stack
kernel_stk: