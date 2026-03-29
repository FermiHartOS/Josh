; boot.asm - Fermi Hart OS Pro
MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; boot.asm - Versão Ultra-Compatível
section .multiboot
align 4
    dd 0x1BADB002               ; Magic number
    dd 0x01                     ; Flags (apenas ALIGN_MODULES para máxima compatibilidade)
    dd -(0x1BADB002 + 0x01)     ; Checksum

section .text
global _start
extern kernel_main

_start:

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB
stack_top:

section .text
global _start
global gdt_flush
extern kernel_main

_start:
    mov esp, stack_top
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang

; Função para carregar a GDT (chamada pelo C)
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10      ; 0x10 é o offset do descritor de dados
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush   ; 0x08 é o offset do descritor de código
.flush:
    ret
