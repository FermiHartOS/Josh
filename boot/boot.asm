;  ================================================================================
; |  JOSH OS - THE REACTIVE MONOLITHIC KERNEL                                    |
; |  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>                        |
; |================================================================================|
;
; boot/boot.asm - Multiboot entry point
;
; GRUB/QEMU coloca em EAX o magic (0x2BADB002) e em EBX o ponteiro
; para a struct multiboot_info. Precisamos passar ambos para kernel_main.

MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC
    dd MBOOT_HEADER_FLAGS
    dd MBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb 32768 ; 32KB Stack
stack_top:

section .text
global _start
global gdt_flush
extern kernel_main

_start:
    mov esp, stack_top

    ; GRUB passa: EAX = multiboot magic, EBX = multiboot info pointer
    ; Convenção cdecl: argumentos na stack da direita para esquerda
    push ebx            ; arg1: multiboot_info_t* mb_info
    push eax            ; arg0: uint32_t mb_magic
    call kernel_main
    add esp, 8          ; Limpa stack (2 args * 4 bytes)

    cli
.hang:
    hlt
    jmp .hang

; Função para carregar a GDT
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush_code

.flush_code:
    ret
