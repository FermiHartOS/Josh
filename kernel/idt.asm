;  ================================================================================
; |  JOSH OS - THE REACTIVE MONOLITHIC KERNEL                                    |
; |  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>                        |
; |================================================================================|
;
; kernel/idt.asm

global idt_load
global irq_handler_keyboard
global irq_handler_timer
global exception_div_zero

extern keyboard_interrupt_handler
extern timer_tick_handler

section .text

; --- Carrega a IDT ---
idt_load:
    cli
    mov eax, [esp + 4]
    lidt [eax]
    sti
    ret

; --- Handler de Exceção: Divisão por Zero ---
exception_div_zero:
    cli
    mov edi, 0xB8000
    mov word [edi + 0],  0x4F44  ; 'D'
    mov word [edi + 2],  0x4F49  ; 'I'
    mov word [edi + 4],  0x4F56  ; 'V'
    mov word [edi + 6],  0x4F30  ; '0'
    mov word [edi + 8],  0x4F21  ; '!'
    hlt
    jmp $

; --- Handler do Teclado (IRQ1 -> Vetor 0x21) ---
irq_handler_keyboard:
    pushad
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call keyboard_interrupt_handler

    mov al, 0x20
    out 0x20, al
    popad
    iret

; --- Handler do Timer (IRQ0 -> Vetor 0x20) ---
; Agora chama timer_tick_handler() em C para contar ticks
irq_handler_timer:
    pushad
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call timer_tick_handler

    mov al, 0x20
    out 0x20, al
    popad
    iret
