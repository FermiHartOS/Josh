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
 * @file drivers/vga/vga.c
 * @brief VGA Text Mode Driver — Hollywood Edition
 *
 * 80x25 · 16 cores · Codepage 437
 *
 * SCROLL PROTEGIDO:
 *   Linhas 0-22: área de conteúdo (scroll normal)
 *   Linha 23:    separador (reservada, nunca rola)
 *   Linha 24:    status bar (reservada, nunca rola)
 *
 *   vga_put_char / vga_put_string nunca escrevem além da linha 22.
 *   Quando cursor_y >= SHELL_MAX_ROW, o scroll move apenas 0-22.
 */

#include <vga.h>

/* ── Hardware ─────────────────────────────────────────────────── */
static uint16_t* const VGA_BUFFER = (uint16_t*)0xB8000;

/* ── Estado ───────────────────────────────────────────────────── */
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = 0x0F;

/* Limite do scroll: linhas 0 até SHELL_MAX_ROW-1 são conteúdo.
 * Linhas SHELL_MAX_ROW..24 são reservadas (status bar). */
#define SHELL_MAX_ROW 23

/* ── I/O ──────────────────────────────────────────────────────── */
static void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ══════════════════════════════════════════════════════════════════
 *  Cursor Hardware
 * ══════════════════════════════════════════════════════════════════ */
void vga_update_cursor(int x, int y) {
    uint16_t pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_hide_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void vga_show_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (0 << 5) | 14);
    outb(0x3D4, 0x0B);
    outb(0x3D5, 15);
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    vga_update_cursor(x, y);
}

/* ══════════════════════════════════════════════════════════════════
 *  Cor global
 * ══════════════════════════════════════════════════════════════════ */
void vga_set_color(uint8_t color) {
    current_color = color;
}

/* ══════════════════════════════════════════════════════════════════
 *  Operações básicas
 * ══════════════════════════════════════════════════════════════════ */
void vga_clear(void) {
    /* Limpa APENAS as linhas de conteúdo (0 a SHELL_MAX_ROW-1) */
    for (int i = 0; i < SHELL_MAX_ROW * 80; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | (uint16_t)0x07 << 8;
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor(0, 0);
}

void vga_clear_color(uint8_t bg_color) {
    uint16_t attr = (uint16_t)(bg_color << 4) << 8;
    for (int i = 0; i < 80 * 25; i++) {
        VGA_BUFFER[i] = (uint16_t)' ' | attr;
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_update_cursor(0, 0);
}

/**
 * @brief Scroll protegido — só rola linhas 0 a SHELL_MAX_ROW-1.
 *        Linhas SHELL_MAX_ROW..24 (status bar) ficam intocadas.
 */
void vga_scroll(void) {
    /* Move linhas 1..(SHELL_MAX_ROW-1) uma linha pra cima */
    for (int i = 0; i < (SHELL_MAX_ROW - 1) * 80; i++)
        VGA_BUFFER[i] = VGA_BUFFER[i + 80];

    /* Limpa a última linha de conteúdo (SHELL_MAX_ROW - 1) */
    for (int x = 0; x < 80; x++)
        VGA_BUFFER[(SHELL_MAX_ROW - 1) * 80 + x] = (uint16_t)' ' | (uint16_t)current_color << 8;

    cursor_y = SHELL_MAX_ROW - 1;
}

void vga_put_char(char c, uint8_t color) {
    uint8_t col = (color == 0) ? current_color : color;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_BUFFER[cursor_y * 80 + cursor_x] = (uint16_t)' ' | (uint16_t)col << 8;
        }
    } else {
        VGA_BUFFER[cursor_y * 80 + cursor_x] = (uint16_t)(uint8_t)c | (uint16_t)col << 8;
        cursor_x++;
    }

    if (cursor_x >= 80) { cursor_x = 0; cursor_y++; }
    if (cursor_y >= SHELL_MAX_ROW) vga_scroll();
    vga_update_cursor(cursor_x, cursor_y);
}

void vga_put_string(const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++)
        vga_put_char(str[i], color);
}

/* ══════════════════════════════════════════════════════════════════
 *  Posicionamento direto (NÃO move o cursor global)
 *  Estas funções PODEM escrever nas linhas 23-24 (status bar).
 * ══════════════════════════════════════════════════════════════════ */
void vga_put_at(int x, int y, uint8_t c, uint8_t color) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    VGA_BUFFER[y * 80 + x] = (uint16_t)c | (uint16_t)color << 8;
}

void vga_put_string_at(int x, int y, const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0' && (x + i) < 80; i++) {
        vga_put_at(x + i, y, (uint8_t)str[i], color);
    }
}

void vga_put_raw(int x, int y, uint16_t entry) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    VGA_BUFFER[y * 80 + x] = entry;
}

/* ══════════════════════════════════════════════════════════════════
 *  Preenchimento
 * ══════════════════════════════════════════════════════════════════ */
void vga_fill_row(int row, uint8_t c, uint8_t color) {
    if (row < 0 || row >= 25) return;
    uint16_t entry = (uint16_t)c | (uint16_t)color << 8;
    for (int x = 0; x < 80; x++)
        VGA_BUFFER[row * 80 + x] = entry;
}

void vga_fill_screen(uint8_t c, uint8_t color) {
    uint16_t entry = (uint16_t)c | (uint16_t)color << 8;
    for (int i = 0; i < 80 * 25; i++)
        VGA_BUFFER[i] = entry;
}

void vga_fill_region(int x, int y, int w, int h, uint8_t c, uint8_t color) {
    uint16_t entry = (uint16_t)c | (uint16_t)color << 8;
    for (int row = y; row < y + h && row < 25; row++)
        for (int col = x; col < x + w && col < 80; col++)
            if (col >= 0 && row >= 0)
                VGA_BUFFER[row * 80 + col] = entry;
}

/* ══════════════════════════════════════════════════════════════════
 *  Box drawing (bordas CP437)
 * ══════════════════════════════════════════════════════════════════ */
void vga_draw_box(int x, int y, int w, int h, uint8_t color) {
    vga_put_at(x,         y,         0xC9, color);
    vga_put_at(x + w - 1, y,         0xBB, color);
    vga_put_at(x,         y + h - 1, 0xC8, color);
    vga_put_at(x + w - 1, y + h - 1, 0xBC, color);
    for (int i = 1; i < w - 1; i++) {
        vga_put_at(x + i, y,         0xCD, color);
        vga_put_at(x + i, y + h - 1, 0xCD, color);
    }
    for (int i = 1; i < h - 1; i++) {
        vga_put_at(x,         y + i, 0xBA, color);
        vga_put_at(x + w - 1, y + i, 0xBA, color);
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  Delay / Efeitos visuais
 * ══════════════════════════════════════════════════════════════════ */
void vga_delay(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 5000; i++)
        asm volatile("nop");
}

void vga_typewriter(const char* str, uint8_t color, uint32_t char_delay_ms) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_put_char(str[i], color);
        if (str[i] != ' ' && str[i] != '\n')
            vga_delay(char_delay_ms);
    }
}

void vga_reveal_row(int row, const char* str, uint8_t color, uint32_t char_delay_ms) {
    for (int i = 0; str[i] != '\0' && i < 80; i++) {
        vga_put_at(i, row, (uint8_t)str[i], color);
        vga_delay(char_delay_ms);
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  Init
 * ══════════════════════════════════════════════════════════════════ */
void vga_init(void) {
    vga_clear();
    vga_show_cursor();
}
