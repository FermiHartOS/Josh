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
 * @file kernel/timer.c
 * @brief PIT Timer + Tick Counter + Uptime Clock
 *
 * O PIT (8253/8254) channel 0 é configurado para gerar IRQ0
 * na frequência desejada (padrão 100 Hz = 10ms por tick).
 *
 * Cada tick incrementa um contador global. A partir dele
 * calculamos segundos, minutos, horas de uptime.
 */

#include <timer.h>

/* ── Estado global ────────────────────────────────────────────── */
static volatile uint32_t tick_count = 0;
static uint32_t timer_freq = 100; /* Hz — ticks por segundo */

/* ── I/O ──────────────────────────────────────────────────────── */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ══════════════════════════════════════════════════════════════════
 *  timer_init() — Configura o PIT channel 0
 *
 *  O PIT roda a 1.193182 MHz internamente.
 *  Divisor = 1193182 / frequência_desejada
 *
 *  100 Hz → divisor = 11931 → tick a cada 10ms
 * ══════════════════════════════════════════════════════════════════ */
void timer_init(uint32_t frequency) {
    timer_freq = frequency;
    uint32_t divisor = 1193182 / frequency;

    /* Command byte: channel 0, lobyte/hibyte, mode 3 (square wave) */
    outb(0x43, 0x36);

    /* Envia divisor (low byte primeiro, depois high byte) */
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

/* ══════════════════════════════════════════════════════════════════
 *  timer_tick_handler() — Chamado pelo IRQ0 handler em idt.asm
 * ══════════════════════════════════════════════════════════════════ */
void timer_tick_handler(void) {
    tick_count++;
}

/* ══════════════════════════════════════════════════════════════════
 *  Getters
 * ══════════════════════════════════════════════════════════════════ */
uint32_t timer_get_ticks(void) {
    return tick_count;
}

uint32_t timer_get_seconds(void) {
    return tick_count / timer_freq;
}

uint32_t timer_get_uptime_hours(void) {
    return timer_get_seconds() / 3600;
}

uint32_t timer_get_uptime_minutes(void) {
    return (timer_get_seconds() % 3600) / 60;
}

uint32_t timer_get_uptime_seconds(void) {
    return timer_get_seconds() % 60;
}

void timer_get_clock(uint32_t* hours, uint32_t* minutes, uint32_t* secs) {
    uint32_t total = timer_get_seconds();
    *hours   = (total / 3600) % 24;
    *minutes = (total % 3600) / 60;
    *secs    = total % 60;
}
