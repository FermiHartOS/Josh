/* user/apps/test_alloc.c */
#include <stdint.h>
#include <vga.h>
#include <mm.h>

// Função principal do app
int test_alloc_main(void) {
    vga_put_string("\n[test_alloc] INICIANDO TESTES DE MEMÓRIA...\n", COLOR_YELLOW);

    // --- TESTE 1 ---
    vga_put_string("[TEST 1] Alocando 32 bytes...", COLOR_WHITE);
    uint8_t* ptr_small = (uint8_t*)kmalloc(32);
    
    if (ptr_small == NULL) {
        vga_put_string("FALHA: kmalloc retornou NULL!\n", COLOR_LIGHT_RED);
        return 1;
    }
    vga_put_string("SUCESSO!\n", COLOR_GREEN);
    
    for(int i=0; i<32; i++) ptr_small[i] = (uint8_t)i;
    
    int ok = 1;
    for(int i=0; i<32; i++) if(ptr_small[i] != (uint8_t)i) ok = 0;
    
    if(ok) vga_put_string("      Integridade: OK\n", COLOR_GREEN);
    else vga_put_string("      Integridade: FALHA!\n", COLOR_LIGHT_RED);

    kfree(ptr_small);
    vga_put_string("      Liberado.\n", COLOR_DARK_GREY);

    vga_put_string("\n[test_alloc] TODOS OS TESTES CONCLUÍDOS!\n", COLOR_LIGHT_CYAN);
    return 0;
}
