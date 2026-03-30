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

#include <stdint.h>
#include <vga.h>
#include <mm.h> // Para kmalloc/kfree se necessário futuramente

/**
 * @brief Estrutura de Cabeçalho do App Flat Binary
 * 
 * Simplificação: Por enquanto, vamos assumir que o app é linkado diretamente
 * no kernel (estático) mas com uma convenção de nomeação clara.
 * 
 * Futuro: Este header virará parte do arquivo .bin carregado do disco.
 */
typedef struct {
    uint32_t magic;      // Assinatura para validação (ex: 0x4A4F5348 = "jOSH")
    uint32_t entry_point; // Endereço da função main()
    uint32_t size;       // Tamanho total do binário em bytes
} app_header_t;

#define APP_MAGIC 0x4A4F5348 // "jOSH"

// Lista de apps registrados (apontadores para os headers)
// No futuro, isso virá de um filesystem ou tabela de símbolos
static app_header_t* registered_apps[16];
static int app_count = 0;

/**
 * @brief Registra um app no sistema
 * @param name Nome do app (para lookup)
 * @param header Ponteiro para o cabeçalho do app
 */
void app_register(const char* name, app_header_t* header) {
    if (app_count >= 16) {
        vga_put_string("[LOADER] Erro: Lista de apps cheia.\n", COLOR_LIGHT_RED);
        return;
    }
    // Aqui poderíamos verificar se o nome já existe
    registered_apps[app_count++] = header;
    vga_put_string("[LOADER] App registrado: ", COLOR_GREEN);
    vga_put_string(name, COLOR_WHITE);
    vga_put_char('\n', COLOR_WHITE);
}

/**
 * @brief Encontra o header de um app pelo nome
 */
static app_header_t* app_find(const char* name) {
    for (int i = 0; i < app_count; i++) {
        // Comparação simples de strings (assumindo que o nome está no header ou mapeado)
        // Para simplificar agora, vamos usar uma abordagem direta de ponteiros
        // Num sistema real, o header teria um campo 'name' ou usariamos uma hash table.
        // Como estamos linkando estaticamente, vamos confiar na ordem ou em macros.
        
        // Hack temporário para demo: Se o nome for "test_alloc", retorna o primeiro, etc.
        // Melhor: Adicionar uma string de nome dentro da struct app_header_t no assembly/linker script.
        
        // Implementação robusta futura: Usar uma tabela de símbolos gerada pelo linker.
        // Por enquanto, vamos assumir que o app foi chamado via pointer direto.
    }
    return NULL;
}

/**
 * @brief Executa um app dado seu endereço de entrada
 * @param entry_func Ponteiro para a função main do app
 * @return 0 se sucesso, -1 se falha
 */
int app_execute(uint32_t (*entry_func)(void)) {
    if (!entry_func) {
        vga_put_string("[LOADER] Falha: Função de entrada inválida.\n", COLOR_LIGHT_RED);
        return -1;
    }

    vga_put_string("[LOADER] Iniciando aplicação... ", COLOR_CYAN);
    
    // Salva o estado atual se necessário (futuro: contexto do shell)
    
    // Chama a função do app
    // Nota: O app deve retornar um status code
    int ret = entry_func();
    
    vga_put_string("App terminou com código: ", COLOR_DARK_GREY);
    char buf[4];
    itoa(ret, buf);
    vga_put_string(buf, COLOR_WHITE);
    vga_put_char('\n', COLOR_WHITE);
    
    return ret;
}
