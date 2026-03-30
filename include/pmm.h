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

#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

/**
 * @brief Inicializa o Physical Memory Manager.
 * @param total_kb   Total de KB de RAM acima de 1MB (do multiboot mem_upper).
 * @param kernel_end Endereço do fim do kernel (símbolo _kernel_end do linker).
 *
 * O bitmap é colocado logo após _kernel_end.
 * Páginas do kernel e do próprio bitmap são marcadas como usadas.
 */
void pmm_init(uint32_t total_kb, uint32_t kernel_end);

/**
 * @brief Aloca uma página física de 4KB.
 * @return Endereço físico da página, ou 0 se não houver memória.
 */
uint32_t pmm_alloc_page(void);

/**
 * @brief Libera uma página física de 4KB.
 * @param addr Endereço físico da página (deve ser alinhado a 4KB).
 */
void pmm_free_page(uint32_t addr);

/* ── Estatísticas ─────────────────────────────────────────────── */
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_used_pages(void);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_kb(void);
uint32_t pmm_get_free_kb(void);

#endif /* PMM_H */
