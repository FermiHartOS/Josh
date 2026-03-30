/*
 *  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>
 *  PROJECT: jOSh - Operating System
 */

#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/* ── Cores VGA (foreground) ───────────────────────────────────── */
#define COLOR_BLACK         0
#define COLOR_BLUE          1
#define COLOR_GREEN         2
#define COLOR_CYAN          3
#define COLOR_RED           4
#define COLOR_MAGENTA       5
#define COLOR_BROWN         6
#define COLOR_LIGHT_GREY    7
#define COLOR_DARK_GREY     8
#define COLOR_LIGHT_BLUE    9
#define COLOR_LIGHT_GREEN   10
#define COLOR_LIGHT_CYAN    11
#define COLOR_LIGHT_RED     12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW        14
#define COLOR_WHITE         15

/* ── Macro: combina foreground + background em um atributo ──── */
#define VGA_COLOR(fg, bg) ((uint8_t)((bg) << 4 | (fg)))

/* ── Constantes de tela ───────────────────────────────────────── */
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

/* ── CP437 block characters (para arte ASCII) ─────────────────── */
#define CP437_FULL_BLOCK   0xDB  /* █ */
#define CP437_DARK_SHADE   0xB2  /* ▓ */
#define CP437_MEDIUM_SHADE 0xB1  /* ▒ */
#define CP437_LIGHT_SHADE  0xB0  /* ░ */
#define CP437_UPPER_HALF   0xDF  /* ▀ */
#define CP437_LOWER_HALF   0xDC  /* ▄ */

/* ── Funções básicas ──────────────────────────────────────────── */
void vga_init(void);
void vga_clear(void);
void vga_clear_color(uint8_t bg_color);
void vga_scroll(void);
void vga_set_color(uint8_t color);
void vga_put_char(char c, uint8_t color);
void vga_put_string(const char* str, uint8_t color);

/* ── Cursor ───────────────────────────────────────────────────── */
void vga_update_cursor(int x, int y);
void vga_set_cursor(int x, int y);
void vga_hide_cursor(void);
void vga_show_cursor(void);

/* ── Posicionamento direto (não move cursor global) ───────────── */
/*    uint8_t c — aceita CP437 codes 0x00-0xFF sem overflow       */
void vga_put_at(int x, int y, uint8_t c, uint8_t color);
void vga_put_string_at(int x, int y, const char* str, uint8_t color);
void vga_put_raw(int x, int y, uint16_t entry);

/* ── Preenchimento ────────────────────────────────────────────── */
void vga_fill_row(int row, uint8_t c, uint8_t color);
void vga_fill_screen(uint8_t c, uint8_t color);
void vga_fill_region(int x, int y, int w, int h, uint8_t c, uint8_t color);

/* ── Box drawing (bordas CP437) ───────────────────────────────── */
void vga_draw_box(int x, int y, int w, int h, uint8_t color);

/* ── Efeitos visuais ──────────────────────────────────────────── */
void vga_delay(uint32_t ms);
void vga_typewriter(const char* str, uint8_t color, uint32_t char_delay_ms);
void vga_reveal_row(int row, const char* str, uint8_t color, uint32_t char_delay_ms);

#endif /* VGA_H */
