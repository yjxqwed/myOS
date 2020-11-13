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
    mov esp, boot_stk - kernel_space_base_addr
    mov ebp, esp

    push eax
    push ebx
    call entry_setup
    add esp, 8
    ; jmp to the higher half
    lea eax, [paging_relocation]
    jmp eax
paging_relocation:
    add esp, kernel_space_base_addr
    add ebp, kernel_space_base_addr
    call ksetup

    ; setup done, jmp to the kernel
    jmp kernel_entry


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

global flushPD
extern _pd  ; page directory base address
flushPD:
    ; _pd is vaddr, we need paddr here
    mov eax, [_pd - kernel_space_base_addr]
    mov cr3, eax

    mov eax, cr0  ; enable paging
    or eax, 0x80000000  ; bit 31 of cr0 is to enable paging
    mov cr0, eax

    ret

global flush_boot_pd
extern _boot_pd
flush_boot_pd:
    mov eax, [_boot_pd - kernel_space_base_addr]
    mov cr3, eax
    mov eax, cr0  ; enable paging
    or eax, 0x80000000  ; bit 31 of cr0 is to enable paging
    mov cr0, eax
    ret


extern kernelMain
; start of the kernel
kernel_entry:
    call kernelMain
    jmp $

[section .data]
times 1024 dw 0
boot_stk:
