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
 * @file kernel/pmm.c
 * @brief Physical Memory Manager — Bitmap Allocator
 *
 * Gerencia páginas físicas de 4KB usando um bitmap.
 * Cada bit = 1 página: 0 = livre, 1 = usada.
 *
 * Layout na memória:
 *   0x000000 - 0x0FFFFF : Reservado (BIOS, VGA, ROM) — marcado como usado
 *   0x100000 - _kernel_end : Kernel code/data/bss — marcado como usado
 *   _kernel_end - bitmap_end : O próprio bitmap — marcado como usado
 *   bitmap_end - RAM_TOP : Memória livre disponível para alocação
 *
 * O QEMU com -m 128 dá ~128MB = ~32768 páginas = bitmap de 4KB.
 */

#include <pmm.h>

/* ── Estado ───────────────────────────────────────────────────── */
static uint8_t* bitmap     = 0;      /* Ponteiro para o bitmap     */
static uint32_t total_pages = 0;     /* Total de páginas gerenciadas */
static uint32_t used_pages  = 0;     /* Páginas marcadas como usadas */
static uint32_t bitmap_size = 0;     /* Tamanho do bitmap em bytes  */

/* ── Helpers de bitmap ────────────────────────────────────────── */
static inline void bitmap_set(uint32_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static inline void bitmap_clear(uint32_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline int bitmap_test(uint32_t page) {
    return (bitmap[page / 8] >> (page % 8)) & 1;
}

/* ══════════════════════════════════════════════════════════════════
 *  pmm_init()
 *
 *  @param total_kb    KB de RAM acima de 1MB (multiboot mem_upper)
 *  @param kernel_end  Endereço de _kernel_end (do linker)
 * ══════════════════════════════════════════════════════════════════ */
void pmm_init(uint32_t total_kb, uint32_t kernel_end) {
    /* RAM total = 1MB (baixa) + mem_upper KB */
    uint32_t total_ram = (1024 + total_kb) * 1024; /* em bytes */

    total_pages = total_ram / PAGE_SIZE;
    bitmap_size = (total_pages + 7) / 8; /* bytes necessários */

    /* Bitmap fica logo após o kernel, alinhado a 4 bytes */
    bitmap = (uint8_t*)((kernel_end + 3) & ~3);

    /* Marca TUDO como usado inicialmente (safe default) */
    for (uint32_t i = 0; i < bitmap_size; i++)
        bitmap[i] = 0xFF;
    used_pages = total_pages;

    /* Libera páginas acima do bitmap até o topo da RAM */
    uint32_t bitmap_end = (uint32_t)bitmap + bitmap_size;
    bitmap_end = (bitmap_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); /* Alinha */

    uint32_t first_free_page = bitmap_end / PAGE_SIZE;
    uint32_t last_page = total_pages;

    for (uint32_t p = first_free_page; p < last_page; p++) {
        bitmap_clear(p);
        used_pages--;
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  pmm_alloc_page() — First-fit no bitmap
 * ══════════════════════════════════════════════════════════════════ */
uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            used_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0; /* Sem memória */
}

/* ══════════════════════════════════════════════════════════════════
 *  pmm_free_page()
 * ══════════════════════════════════════════════════════════════════ */
void pmm_free_page(uint32_t addr) {
    uint32_t page = addr / PAGE_SIZE;
    if (page < total_pages && bitmap_test(page)) {
        bitmap_clear(page);
        used_pages--;
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  Estatísticas
 * ══════════════════════════════════════════════════════════════════ */
uint32_t pmm_get_total_pages(void) { return total_pages; }
uint32_t pmm_get_used_pages(void)  { return used_pages; }
uint32_t pmm_get_free_pages(void)  { return total_pages - used_pages; }
uint32_t pmm_get_total_kb(void)    { return total_pages * 4; }
uint32_t pmm_get_free_kb(void)     { return (total_pages - used_pages) * 4; }
