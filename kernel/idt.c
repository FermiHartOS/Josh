/*
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
 */
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
