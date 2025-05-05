bits 32
global start
extern kernel_main

section .multiboot
    align 4
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
start:
    mov esp, stack_top

    ; Call kernel
    call kernel_main

    ; Hang if kernel returns
    cli
    hlt
.hang:
    jmp .hang

section .bss
    align 16
    stack_bottom:
        resb 16384
    stack_top: