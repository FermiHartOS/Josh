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
 * @brief Entry point do jOSh OS — Hollywood Boot Edition
 *
 * Boot cinematográfico em 4 atos:
 *   Ato 0: Splash image (alien avatar) — impacto visual imediato
 *   Ato 1: Logo jOSh + info box com efeitos
 *   Ato 2: Boot log estilo Hollywood (subsistemas com [OK])
 *   Ato 3: Shell interativo
 */

#include <stdint.h>
#include <vga.h>
#include <keyboard.h>
#include <shell.h>
#include <gdt.h>
#include <idt.h>
#include <nosound.h>
#include <splash.h>  /* Array splash_image[2000] gerado da foto */

/* ── Acesso direto ao VGA buffer para blit rápido ─────────── */
static uint16_t* const VGA_RAW = (uint16_t*)0xB8000;

/* ══════════════════════════════════════════════════════════════════
 *  Utilitários internos do boot
 * ══════════════════════════════════════════════════════════════════ */

/**
 * @brief Blit da imagem splash direto no VGA buffer.
 *        Efeito: reveal linha por linha com delay (cortina descendo).
 */
static void blit_splash_animated(void) {
    vga_hide_cursor();

    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            VGA_RAW[y * 80 + x] = splash_image[y * 80 + x];
        }
        vga_delay(30); /* Cortina: cada linha aparece com delay */
    }
}

/**
 * @brief Overlay de texto sobre a splash (semi-transparente).
 *        Escreve texto sem apagar o fundo — usa a cor do pixel como BG.
 */
static void splash_overlay_text(int x, int y, const char* str, uint8_t fg) {
    for (int i = 0; str[i] != '\0' && (x + i) < 80; i++) {
        /* Pega a cor de background do pixel que já está lá */
        uint16_t existing = VGA_RAW[y * 80 + (x + i)];
        uint8_t existing_attr = (existing >> 8) & 0xFF;
        uint8_t existing_fg = existing_attr & 0x0F;

        /* Usa a cor dominante do pixel existente como background */
        uint8_t new_attr = (existing_fg << 4) | fg;
        VGA_RAW[y * 80 + (x + i)] = (uint16_t)str[i] | ((uint16_t)new_attr << 8);
    }
}

/**
 * @brief Progress bar animada.
 */
static void boot_progress_bar(int row, uint8_t color) {
    int bar_x = 20;
    int bar_w = 40;

    vga_put_at(bar_x - 1, row, '[', COLOR_DARK_GREY);
    vga_put_at(bar_x + bar_w, row, ']', COLOR_DARK_GREY);

    for (int i = 0; i < bar_w; i++) {
        vga_put_at(bar_x + i, row, 0xDB, color);
        vga_delay(8);
    }
}

/**
 * @brief Boot step com [  OK  ] visual.
 */
static void boot_step(const char* msg, uint8_t msg_color) {
    vga_put_string("  [", COLOR_DARK_GREY);
    vga_put_string("  OK  ", COLOR_LIGHT_GREEN);
    vga_put_string("] ", COLOR_DARK_GREY);
    vga_put_string(msg, msg_color);
    vga_put_char('\n', COLOR_WHITE);
    vga_delay(80);
}

/**
 * @brief Espera qualquer tecla via IRQ1 buffer.
 */
static void wait_any_key(void) {
    /* Drena lixo */
    while (get_key_from_buffer() != 0) { }
    /* Espera tecla nova */
    while (get_key_from_buffer() == 0) {
        asm volatile("hlt");
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  ATO 0 — Splash Image (primeiro impacto visual)
 * ══════════════════════════════════════════════════════════════════ */
static void act0_splash_image(void) {
    /* Tela preta por um instante (tensão) */
    vga_clear_color(COLOR_BLACK);
    vga_hide_cursor();
    vga_delay(400);

    /* Revela a imagem linha por linha */
    blit_splash_animated();

    /* Overlay de texto sobre a imagem */
    vga_delay(300);
    splash_overlay_text(28, 1, "F E R M I  *  H A R T", COLOR_WHITE);
    splash_overlay_text(30, 22, "j O S h   O S   v1.3", COLOR_WHITE);
    splash_overlay_text(21, 24, "Press any key to initialize...", COLOR_YELLOW);
}

/* ══════════════════════════════════════════════════════════════════
 *  ATO 1 — Logo jOSh + Info Box
 * ══════════════════════════════════════════════════════════════════ */
static void act1_logo(void) {
    vga_clear_color(COLOR_BLACK);
    vga_hide_cursor();
    vga_delay(200);

    uint8_t logo_hi = COLOR_LIGHT_CYAN;
    uint8_t logo_lo = COLOR_CYAN;

    /* "j" */
    int jx = 15;
    vga_put_at(jx+4, 1, 0xFE, logo_hi);
    vga_put_at(jx+4, 3, 0xDB, logo_hi);
    vga_put_at(jx+4, 4, 0xDB, logo_hi);
    vga_put_at(jx+4, 5, 0xDB, logo_hi);
    vga_put_at(jx+4, 6, 0xDB, logo_hi);
    vga_put_at(jx+1, 7, 0xDB, logo_lo);
    vga_put_at(jx+2, 7, 0xDB, logo_hi);
    vga_put_at(jx+3, 7, 0xDB, logo_hi);
    vga_put_at(jx+4, 7, 0xDB, logo_hi);
    vga_put_at(jx+2, 8, 0xDB, logo_lo);
    vga_put_at(jx+3, 8, 0xDB, logo_lo);
    vga_delay(100);

    /* "O" */
    int ox = 25;
    vga_put_at(ox+1, 3, 0xDB, logo_hi); vga_put_at(ox+2, 3, 0xDB, logo_hi); vga_put_at(ox+3, 3, 0xDB, logo_hi);
    vga_put_at(ox+0, 4, 0xDB, logo_hi); vga_put_at(ox+4, 4, 0xDB, logo_hi);
    vga_put_at(ox+0, 5, 0xDB, logo_hi); vga_put_at(ox+4, 5, 0xDB, logo_hi);
    vga_put_at(ox+0, 6, 0xDB, logo_hi); vga_put_at(ox+4, 6, 0xDB, logo_hi);
    vga_put_at(ox+1, 7, 0xDB, logo_lo); vga_put_at(ox+2, 7, 0xDB, logo_lo); vga_put_at(ox+3, 7, 0xDB, logo_lo);
    vga_delay(100);

    /* "S" */
    int sxx = 35;
    vga_put_at(sxx+1,3,0xDB,logo_hi); vga_put_at(sxx+2,3,0xDB,logo_hi); vga_put_at(sxx+3,3,0xDB,logo_hi); vga_put_at(sxx+4,3,0xDB,logo_hi);
    vga_put_at(sxx+0,4,0xDB,logo_hi);
    vga_put_at(sxx+1,5,0xDB,logo_hi); vga_put_at(sxx+2,5,0xDB,logo_hi); vga_put_at(sxx+3,5,0xDB,logo_hi);
    vga_put_at(sxx+4,6,0xDB,logo_hi);
    vga_put_at(sxx+0,7,0xDB,logo_lo); vga_put_at(sxx+1,7,0xDB,logo_lo); vga_put_at(sxx+2,7,0xDB,logo_lo); vga_put_at(sxx+3,7,0xDB,logo_lo);
    vga_delay(100);

    /* "h" */
    int hxx = 45;
    vga_put_at(hxx+0,2,0xDB,logo_hi); vga_put_at(hxx+0,3,0xDB,logo_hi);
    vga_put_at(hxx+0,4,0xDB,logo_hi); vga_put_at(hxx+1,4,0xDB,logo_hi); vga_put_at(hxx+2,4,0xDB,logo_hi); vga_put_at(hxx+3,4,0xDB,logo_hi);
    vga_put_at(hxx+0,5,0xDB,logo_hi); vga_put_at(hxx+4,5,0xDB,logo_hi);
    vga_put_at(hxx+0,6,0xDB,logo_hi); vga_put_at(hxx+4,6,0xDB,logo_hi);
    vga_put_at(hxx+0,7,0xDB,logo_lo); vga_put_at(hxx+4,7,0xDB,logo_lo);
    vga_delay(100);

    /* Infinito */
    vga_put_at(55, 5, 0xEC, COLOR_LIGHT_MAGENTA);
    vga_delay(200);

    /* Autor */
    vga_put_string_at(21, 10, "F E R M I     H A R T", COLOR_DARK_GREY);
    vga_put_at(32, 10, 0xEC, COLOR_LIGHT_MAGENTA);
    vga_delay(200);

    /* Separador */
    for (int i = 10; i < 70; i++) {
        vga_put_at(i, 11, 0xC4, COLOR_DARK_GREY);
        vga_delay(3);
    }

    /* Info box */
    vga_draw_box(12, 13, 56, 7, COLOR_CYAN);
    vga_put_string_at(15, 14, "jOSh OS v1.3", COLOR_WHITE);
    vga_put_string_at(42, 14, "Reactive Monolithic", COLOR_DARK_GREY);
    vga_put_string_at(15, 15, "Arch:", COLOR_DARK_GREY);
    vga_put_string_at(21, 15, "x86_32 Protected Mode", COLOR_LIGHT_GREEN);
    vga_put_string_at(15, 16, "GDT:", COLOR_DARK_GREY);
    vga_put_string_at(21, 16, "Flat 4GB", COLOR_LIGHT_GREEN);
    vga_put_string_at(32, 16, "IDT:", COLOR_DARK_GREY);
    vga_put_string_at(37, 16, "256 gates, PIC remapped", COLOR_LIGHT_GREEN);
    vga_put_string_at(15, 17, "IRQs:", COLOR_DARK_GREY);
    vga_put_string_at(21, 17, "Timer(0x20)  Keyboard(0x21)", COLOR_LIGHT_GREEN);
    vga_put_string_at(15, 18, "VGA:", COLOR_DARK_GREY);
    vga_put_string_at(21, 18, "80x25 Direct @ 0xB8000", COLOR_LIGHT_GREEN);
    vga_delay(300);

    vga_put_string_at(16, 21, "\"The code is free. The music is mandatory.\"", COLOR_DARK_GREY);
    vga_put_string_at(23, 23, "Press any key to continue...", COLOR_YELLOW);
}

/* ══════════════════════════════════════════════════════════════════
 *  ATO 2 — Boot Log (estilo Hollywood/systemd)
 * ══════════════════════════════════════════════════════════════════ */
static void act2_boot_log(void) {
    vga_clear_color(COLOR_BLACK);
    vga_hide_cursor();

    /* Header bar */
    vga_fill_row(0, ' ', VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    vga_put_string_at(2, 0, " jOSh OS ", VGA_COLOR(COLOR_WHITE, COLOR_BLUE));
    vga_put_string_at(60, 0, "Boot Sequence ", VGA_COLOR(COLOR_LIGHT_CYAN, COLOR_BLUE));

    vga_set_cursor(0, 2);
    vga_delay(100);

    boot_step("VGA text mode initialized (80x25, 16 colors)", COLOR_LIGHT_GREY);
    boot_step("GDT loaded: Null + Code(0x08) + Data(0x10)", COLOR_LIGHT_GREY);
    boot_step("PIC 8259 remapped: Master=0x20, Slave=0x28", COLOR_LIGHT_GREY);
    boot_step("IDT loaded: 256 entries, 3 active handlers", COLOR_LIGHT_GREY);
    boot_step("IRQ0 Timer handler at vector 0x20", COLOR_LIGHT_GREY);
    boot_step("IRQ1 Keyboard handler at vector 0x21", COLOR_LIGHT_GREY);
    boot_step("Keyboard driver: circular buffer (256B)", COLOR_LIGHT_GREY);
    boot_step("Exception handler: DIV0 at vector 0x00", COLOR_LIGHT_GREY);
    boot_step("Interrupts enabled (STI)", COLOR_LIGHT_GREEN);

    vga_set_cursor(0, 13);
    vga_put_string("  Initializing shell...\n\n", COLOR_DARK_GREY);
    boot_progress_bar(15, COLOR_LIGHT_CYAN);
    vga_delay(100);

    vga_set_cursor(0, 17);
    boot_step("MiniBash Pro shell ready", COLOR_LIGHT_GREEN);
    boot_step("System is REACTIVE. All subsystems nominal.", COLOR_LIGHT_CYAN);
    vga_delay(300);

    vga_put_string_at(23, 21, "Press any key to enter shell...", COLOR_YELLOW);
}

/* ══════════════════════════════════════════════════════════════════
 *  kernel_main() — The Hollywood Boot
 *
 *  Hardware init é silencioso (GDT + IDT acontecem antes de
 *  qualquer visual). O usuário só vê arte e resultados.
 * ══════════════════════════════════════════════════════════════════ */
void kernel_main(void) {

    /* ── Hardware init (silencioso) ─────────────────────────── */
    vga_init();
    speaker_init();
    gdt_install();
    idt_install();

    /* ── ATO 0: Splash com avatar ──────────────────────────── */
    act0_splash_image();
    wait_any_key();

    /* ── ATO 1: Logo jOSh + specs ──────────────────────────── */
    act1_logo();
    wait_any_key();

    /* ── ATO 2: Boot log cinematográfico ───────────────────── */
    act2_boot_log();
    wait_any_key();

    /* ── ATO 3: Shell ──────────────────────────────────────── */
    while (get_key_from_buffer() != 0) { /* drena */ }
    vga_show_cursor();
    start_shell();
}
