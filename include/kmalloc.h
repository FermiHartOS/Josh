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

#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>

/**
 * @brief Inicializa o heap do kernel.
 *        Pede páginas ao PMM e cria a free list inicial.
 * @param initial_pages Número de páginas de 4KB para o heap inicial.
 */
void heap_init(uint32_t initial_pages);

/**
 * @brief Aloca 'size' bytes no heap do kernel.
 *        First-fit com split se o bloco for grande o suficiente.
 * @return Ponteiro para a memória alocada, ou 0 (NULL) se falhar.
 */
void* kmalloc(uint32_t size);

/**
 * @brief Libera memória alocada com kmalloc.
 *        Faz merge com blocos adjacentes livres.
 */
void kfree(void* ptr);

/* ── Estatísticas ─────────────────────────────────────────────── */
uint32_t heap_get_total(void);
uint32_t heap_get_used(void);
uint32_t heap_get_free(void);
uint32_t heap_get_alloc_count(void);

#endif /* KMALLOC_H */
