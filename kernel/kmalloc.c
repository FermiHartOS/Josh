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
 * @file kernel/kmalloc.c
 * @brief Kernel Heap Allocator — First-fit linked list
 *
 * Cada bloco tem um header de 12 bytes:
 *   - size:  tamanho dos dados (sem o header)
 *   - free:  1 = livre, 0 = alocado
 *   - next:  ponteiro para o próximo bloco
 *
 * Operações:
 *   kmalloc: first-fit search, split se sobra >= 16 bytes
 *   kfree:   marca como livre, merge com vizinhos adjacentes
 */

#include <kmalloc.h>
#include <pmm.h>

/* ── Block header ─────────────────────────────────────────────── */
typedef struct block_header {
    uint32_t size;              /* Tamanho dos DADOS (sem header) */
    uint32_t free;              /* 1 = livre, 0 = alocado */
    struct block_header* next;  /* Próximo bloco na lista */
} block_header_t;

#define HEADER_SIZE sizeof(block_header_t)
#define MIN_SPLIT   16  /* Mínimo de dados para justificar split */

/* ── Estado do heap ───────────────────────────────────────────── */
static block_header_t* heap_start = 0;
static uint32_t heap_total   = 0;
static uint32_t heap_used    = 0;
static uint32_t alloc_count  = 0;

/* ══════════════════════════════════════════════════════════════════
 *  heap_init() — Pede páginas ao PMM e cria o bloco inicial
 * ══════════════════════════════════════════════════════════════════ */
void heap_init(uint32_t initial_pages) {
    /* Aloca páginas contíguas do PMM */
    uint32_t first_page = pmm_alloc_page();
    if (first_page == 0) return; /* Sem memória */

    for (uint32_t i = 1; i < initial_pages; i++) {
        pmm_alloc_page(); /* Assume contiguidade (sem paging, é linear) */
    }

    heap_total = initial_pages * PAGE_SIZE;

    /* Cria um único bloco livre cobrindo todo o heap */
    heap_start = (block_header_t*)first_page;
    heap_start->size = heap_total - HEADER_SIZE;
    heap_start->free = 1;
    heap_start->next = 0;
}

/* ══════════════════════════════════════════════════════════════════
 *  kmalloc() — First-fit allocation
 * ══════════════════════════════════════════════════════════════════ */
void* kmalloc(uint32_t size) {
    if (size == 0) return 0;

    /* Alinha a 4 bytes */
    size = (size + 3) & ~3;

    block_header_t* cur = heap_start;

    while (cur) {
        if (cur->free && cur->size >= size) {
            /* Split se sobra espaço suficiente para novo bloco */
            if (cur->size >= size + HEADER_SIZE + MIN_SPLIT) {
                block_header_t* new_block = (block_header_t*)((uint8_t*)cur + HEADER_SIZE + size);
                new_block->size = cur->size - size - HEADER_SIZE;
                new_block->free = 1;
                new_block->next = cur->next;

                cur->size = size;
                cur->next = new_block;
            }

            cur->free = 0;
            heap_used += cur->size;
            alloc_count++;

            /* Retorna ponteiro APÓS o header */
            return (void*)((uint8_t*)cur + HEADER_SIZE);
        }
        cur = cur->next;
    }

    return 0; /* Sem memória */
}

/* ══════════════════════════════════════════════════════════════════
 *  kfree() — Libera e faz merge com vizinhos
 * ══════════════════════════════════════════════════════════════════ */
void kfree(void* ptr) {
    if (!ptr) return;

    block_header_t* block = (block_header_t*)((uint8_t*)ptr - HEADER_SIZE);
    block->free = 1;
    heap_used -= block->size;
    alloc_count--;

    /* Merge com o próximo se também estiver livre */
    if (block->next && block->next->free) {
        block->size += HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }

    /* Merge com o anterior (scan linear desde o início) */
    block_header_t* cur = heap_start;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next = cur->next->next;
        }
        cur = cur->next;
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  Estatísticas
 * ══════════════════════════════════════════════════════════════════ */
uint32_t heap_get_total(void)       { return heap_total; }
uint32_t heap_get_used(void)        { return heap_used; }
uint32_t heap_get_free(void)        { return heap_total - heap_used - HEADER_SIZE; }
uint32_t heap_get_alloc_count(void) { return alloc_count; }
