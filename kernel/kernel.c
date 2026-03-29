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
 * @file kernel/kernel.c
 * @brief Entry point do jOSh OS - Monólito Reativo
 */

#include <stdint.h>
#include <vga.h>
#include <keyboard.h>
#include <shell.h>
#include <gdt.h>
#include <idt.h>

void kernel_main() {
    // --- 1. Instala a GDT (Global Descriptor Table) ---
    // Define os segmentos de memória (Código e Dados) para o Kernel
    gdt_install();
    
    vga_init();
    vga_put_string("jOSh OS [Reactive Monolithic Kernel]\\n", COLOR_CYAN);
    vga_put_string("[GDT] Memory Segments Initialized... OK\\n", COLOR_GREEN);

    // --- 2. Instala a IDT (Interrupt Descriptor Table) ---
    // Habilita interrupções de hardware (Teclado, Timer, etc.)
    idt_install();
    vga_put_string("[IDT] Interrupt Vector Table Installed... OK\\n", COLOR_GREEN);
    vga_put_string("[CPU] Hardware Interrupts ENABLED (STI)... OK\\n", COLOR_LIGHT_GREEN);

    vga_put_string("\\nLoading System Modules: ", COLOR_LIGHT_GREY);
    for(int i = 0; i < 20; i++) {
        vga_put_char('#', COLOR_GREEN);
        // Delay para efeito visual
        for(volatile int d = 0; d < 3000000; d++); 
    }
    
    vga_put_string(" DONE!\\n", COLOR_GREEN);
    vga_put_string("\\nThe system is now REACTIVE. Waiting for events...\\n", COLOR_WHITE);

    // Inicia o Shell interativo
    // O start_shell agora lê do buffer circular preenchido pela IDT
    start_shell();
}
