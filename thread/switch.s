[bits 32]

[section .text]
global switch_to
switch_to:
    push esi
    push edi
    push ebx
    push ebp

    ; xchg bx, bx
    mov eax, [esp + 20]
    mov [eax], esp

    mov eax, [esp + 24]
    mov esp, [eax]

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret