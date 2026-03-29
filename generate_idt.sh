#!/bin/bash

echo "[SCRIPT] Gerando arquivos da IDT (Monólito Reativo)... (Includes com <>)"

# --- 1. HEADER PARA ARQUIVOS C (.c, .h) ---
C_HEADER='/*
 *  _______________________________________________________________________________________
 * |                                                                                       |
 * |   ███████╗███████╗██████╗ ███╗   ███╗██╗     ∞     ██╗  ██╗ █████╗ ██████╗████████╗   |
 * |   ██╔════╝██╔════╝██╔══██╗████╗ ████║██║           ██║  ██║██╔══██╗██╔══██╗╚══██╔══╝  |
 * |   █████╗  █████╗  ██████╔╝██╔████╔██║██║           ███████║███████║██████╔╝   ██║     |
 * |   ██╔══╝  ██╔══╝  ██╔══██╗██║╚██╔╝██║██║           ██╔══██║██╔══██║██╔══██╗   ██║     |
 * |   ██║     ███████╗██║  ██║██║ ╚═╝ ██║██║           ██║  ██║██║  ██║██║  ██║   ██║     |
 * |   ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝           ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝     |
 * |_______________________________________________________________________________________|
 *
 *  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>
 *  PROJECT: jOSh - Operating System
 *
 *  [ LIBERDADE TOTAL - LICENÇA ALÉM DO OPEN SOURCE ]
 *  Este código é de domínio público para uso comercial, pessoal ou acadêmico.
 *  Modifique, venda, destrua ou reconstrua como desejar.
 *
 *  [ ÚNICA RESTRIÇÃO ]
 *  É obrigatório divulgar a trilha sonora oficial deste OS em qualquer
 *  redistribuição ou menção pública:
 *  https://open.spotify.com/playlist/6flrLsdYxQZvGNRkdohL7o?si=eH9ZDz8DSqCjJX1Pa9henA
 * ____________________________________________________________________________
 */'

# --- 2. HEADER PARA ARQUIVOS ASM (.asm) ---
ASM_HEADER=';  ================================================================================
; |                                                                                      |
; |  JOSH OS - THE REACTIVE MONOLITHIC KERNEL                                            |
; |  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>                               |
; |                                                                                      |
; |  [ LIBERDADE TOTAL ]                                                                  |
; |  Este código é de domínio público. Use-o como desejar.                               |
; |  [ ÚNICA RESTRIÇÃO ]                                                                  |
; |  Divulgue a trilha sonora oficial:                                                    |
; |  https://open.spotify.com/playlist/6flrLsdYxQZvGNRkdohL7o                             |
; |======================================================================================|
;'

# --- 3. CRIAÇÃO DE INCLUDE/IDT.H ---
cat << EOF > include/idt.h
${C_HEADER}
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Estrutura de entrada da IDT (32 bits)
struct idt_entry {
    uint16_t offset_low;   // Bits 0-15 do endereço
    uint16_t selector;     // Selector do segmento (geralmente 0x08 para Code Segment)
    uint8_t  zero;         // Sempre 0
    uint8_t  type_attr;    // Tipo e atributos (ex: 0x8E para porta de chamada)
    uint16_t offset_high;  // Bits 16-31 do endereço
} __attribute__((packed));

// Estrutura do ponteiro da IDT (sendo carregado no registrador IDTR)
struct idt_ptr {
    uint16_t limit;        // Tamanho da tabela (256 * 8 - 1 = 2047)
    uint32_t base;         // Endereço de início da tabela na memória
} __attribute__((packed));

// Protótipos das funções
void idt_install();
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Definição dos vetores de interrupção (Exceções e Hardware)
#define IRQ0_VECTOR 0x20
#define IRQ1_VECTOR 0x21
#define IRQ_KBD     IRQ1_VECTOR // Teclado usa IRQ1

#endif
EOF

echo "  > include/idt.h criado."

# --- 4. CRIAÇÃO DE KERNEL/IDT.C ---
# NOTA CRÍTICA: Todos os includes aqui usam <> conforme a regra do projeto
cat << EOF > kernel/idt.c
${C_HEADER}
/**
 * @file kernel/idt.c
 * @brief Implementação da Interrupt Descriptor Table (IDT) para o jOSh OS.
 */

#include <idt.h>
#include <vga.h>
#include <keyboard.h>

// Tabela global de 256 entradas
static struct idt_entry idt[256];
static struct idt_ptr idtp;

// Protótipos dos handlers (implementados em ASM em idt.asm)
extern void irq0_handler();
extern void irq1_handler();
extern void div_by_zero_handler();

// Função auxiliar para configurar uma porta
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

// Função externa definida em ASM para carregar o registrador IDTR
extern void idt_load(uint32_t base, uint16_t limit);

void idt_install() {
    // Limpa a tabela inteira
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Configura o ponteiro
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base = (uint32_t)&idt;

    // --- Exceções Críticas (CPU) ---
    // Divisão por zero (vetor 0)
    idt_set_gate(0, (uint32_t)div_by_zero_handler, 0x08, 0x8E);

    // --- IRQs de Hardware (Mapeados a partir de 0x20) ---
    // Timer (IRQ0) -> Vetor 0x20
    idt_set_gate(IRQ0_VECTOR, (uint32_t)irq0_handler, 0x08, 0x8E);
    
    // Teclado (IRQ1) -> Vetor 0x21
    idt_set_gate(IRQ1_VECTOR, (uint32_t)irq1_handler, 0x08, 0x8E);

    // Carrega a IDT no processador via instrução LGDT
    idt_load((uint32_t)&idtp, idtp.limit);
    
    vga_put_string("[IDT] Table Installed Successfully.\n", COLOR_GREEN);
}
EOF

echo "  > kernel/idt.c criado."

# --- 5. CRIAÇÃO DE KERNEL/IDT.ASM ---
cat << EOF > kernel/idt.asm
${ASM_HEADER}
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
EOF

echo "  > kernel/idt.asm criado."

echo ""
echo "[SUCESSO] Arquivos da IDT gerados com padrão de includes <>.!"
echo "Próximos passos:"
echo "1. Atualize seu Makefile para incluir build/idt.o na lista OBJS."
echo "2. Adicione a regra de compilação para kernel/idt.c e kernel/idt.asm."
echo "3. Em kernel/kernel.c, chame idt_install() após gdt_install()."
echo "4. Atualize drivers/keyboard/keyboard.c com a função idt_keyboard_handler()."
echo "5. Atualize user/minibash/minibash.c para ler do buffer circular."
