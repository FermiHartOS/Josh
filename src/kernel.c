/**
 * @file kernel.c
 * @brief Entry point do Fermi Hart OS Pro
 */

#include <stdint.h>
#include <vga.h>
#include <keyboard.h>
#include <shell.h>

void kernel_main() {
    // Inicializa o hardware de vídeo e limpa a tela
    vga_init();
    
    vga_put_string("Fermi Hart OS Pro [Booting...]\n", COLOR_CYAN);
    
    vga_put_string("Status: Initializing VGA Console... OK\n", COLOR_WHITE);
    vga_put_string("Status: Loading MiniBash Interface... OK\n", COLOR_WHITE);
    
    vga_put_string("\nLoading System Modules: ", COLOR_LIGHT_GREY);
    for(int i = 0; i < 20; i++) {
        vga_put_char('#', COLOR_GREEN);
        // Delay para efeito visual
        for(volatile int d = 0; d < 3000000; d++); 
    }
    
    vga_put_string(" DONE!\n", COLOR_GREEN);

    // Inicia o Shell interativo
    // O start_shell agora assume o controle total
    start_shell();
}
