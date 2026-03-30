; boot.asm - Fermi Hart OS Pro (Versão Estável)
; boot/boot.asm - Versão Estável Final (Com Far Jump Corrigido)
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
    call kernel_main
    
    cli
.hang:
    hlt
    jmp .hang

; Função para carregar a GDT
; Recebe ponteiro para struct gdt_ptr em [esp+4]
gdt_flush:
    mov eax, [esp + 4]      ; Pega endereço do ponteiro GDT
    lgdt [eax]              ; Carrega GDT

    ; Recarrega TODOS os segmentos de dados
    mov ax, 0x10            ; Selector de Dados (Entrada 2 * 8)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; FAR JUMP CRÍTICO: Atualiza CS (Code Segment) para o novo seletor 0x08
    ; Sem isso, a CPU continua usando o CS antigo e trava ao acessar memória
    jmp 0x08:.flush_code

.flush_code:
    ret
