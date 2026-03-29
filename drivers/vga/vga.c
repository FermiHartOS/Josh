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
#include <vga.h>

static uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0x0F;

// Escreve na porta I/O
static void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void vga_update_cursor(int x, int y) {
    uint16_t pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_clear() {
    for (int i = 0; i < 80 * 25; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | (uint16_t)0x07 << 8;
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor(cursor_x, cursor_y);
}

void vga_scroll() {
    for (int i = 0; i < 24 * 80; i++) VGA_BUFFER[i] = VGA_BUFFER[i + 80];
    for (int i = 24 * 80; i < 25 * 80; i++) VGA_BUFFER[i] = (uint16_t)' ' | (uint16_t)current_color << 8;
    cursor_y = 24;
}

void vga_put_char(char c, uint8_t color) {
    // Se a cor passada for 0, usamos a cor global definida por vga_set_color
    uint8_t color_to_use = (color == 0) ? current_color : color;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_BUFFER[cursor_y * 80 + cursor_x] = (uint16_t)' ' | (uint16_t)color_to_use << 8;
        }
    } else {
        VGA_BUFFER[cursor_y * 80 + cursor_x] = (uint16_t)c | (uint16_t)color_to_use << 8;
        cursor_x++;
    }

    if (cursor_x >= 80) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= 25) vga_scroll();
    
    vga_update_cursor(cursor_x, cursor_y);
}


void vga_put_string(const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++) vga_put_char(str[i], color);
}

void vga_init() {
    vga_clear();
}
