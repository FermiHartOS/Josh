;  ================================================================================
; |                                                                                |
; |  JOSH OS - THE REACTIVE MONOLITHIC KERNEL                                      |
; |  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>                         |
; |                                                                                |
; |  [ LIBERDADE TOTAL ]                                                           |
; |  Este código é de domínio público. Use-o como desejar.                         |
; |  [ ÚNICA RESTRIÇÃO ]                                                           |
; |  Divulgue a trilha sonora oficial:                                             |
; |  https://open.spotify.com/playlist/6flrLsdYxQZvGNRkdohL7o                      |
; |================================================================================|
;
; Kernel/idt.asm
; Handlers de Interrupção e Funções Auxiliares

global idt_load
global irq0_handler
global irq1_handler
global div_by_zero_handler

extern idt_keyboard_handler ; Função C que processa a tecla

section .text

; --- Função para carregar a IDT (LGDT) ---
idt_load:
    mov eax, [esp + 4]      ; Base da IDT
    mov ecx, [esp + 8]      ; Limite da IDT
    lgdt [ecx]              ; Carrega o descritor
    sti                     ; Habilita interrupções (INTERRUPT ENABLE)
    ret

; --- Handler de Divisão por Zero (Exceção Crítica) ---
div_by_zero_handler:
    push 0                  ; Fake error code
    push 0                  ; Vector number
    jmp exception_common_stub

; --- Handler do Timer (IRQ0) ---
irq0_handler:
    push 0                  ; Fake error code
    push 0x20               ; Vector number
    jmp interrupt_common_stub

; --- Handler do Teclado (IRQ1) ---
irq1_handler:
    push 0                  ; Fake error code
    push 0x21               ; Vector number
    jmp interrupt_common_stub

; --- Stub Comum para Exceções (Crash) ---
exception_common_stub:
    cli                     ; Desabilita interrupções para não entrar em loop infinito
    mov ah, 0x0F            ; Cor vermelha
    mov al, 'K'
    int 0x10                ; BIOS call para imprimir 'K' (PANIC)
    hlt                     ; Para a CPU
    jmp $

; --- Stub Comum para Interrupções de Hardware ---
interrupt_common_stub:
    pushad                  ; Salva todos os registradores gerais (EAX, EBX, etc.)
    
    push ds                 ; Salva segmentos
    push es
    push fs
    push gs
    
    mov ax, 0x10            ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Pega o número do vetor da pilha (já empurrado antes do jmp)
    mov eax, [esp + 68]     
    cmp eax, 0x21           ; Se for IRQ1 (Teclado)
    je keyboard_interrupt_detected
    
    ; Se for outro (Timer), apenas envia EOI (End of Interrupt)
    mov al, 0x20
    out 0x20, al
    jmp interrupt_end

keyboard_interrupt_detected:
    ; Chama a função C para processar a tecla
    call idt_keyboard_handler
    
    ; Envia EOI ao controlador de interrupções (PIC)
    mov al, 0x20
    out 0x20, al

interrupt_end:
    pop gs
    pop fs
    pop es
    pop ds
    popad                   ; Restaura registradores
    add esp, 8              ; Remove vector e error code da pilha
    iret                    ; Retorna da interrupção (Restaura CS, EIP, EFLAGS)
