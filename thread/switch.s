[bits 32]

[section .text]
global switch_to
switch_to:
    push esi
    push edi
    push ebx
    push ebp

    ; store esp to the current task's pcb
    mov eax, [esp + 20]
    mov [eax], esp

    ; load the next task's esp
    mov eax, [esp + 24]
    mov esp, [eax]

    pop ebp
    pop ebx
    pop edi
    pop esi
    ret
