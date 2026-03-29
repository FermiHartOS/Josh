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
