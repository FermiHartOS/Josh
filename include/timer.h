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

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief Configura o PIT (Programmable Interval Timer) channel 0.
 *        Frequência padrão: 100 Hz (10ms por tick).
 *        Chamado dentro de idt_install().
 */
void timer_init(uint32_t frequency);

/**
 * @brief Chamado pelo handler ASM do IRQ0 a cada tick.
 *        Incrementa contador global e atualiza tempo.
 */
void timer_tick_handler(void);

/* ── Getters de tempo (calculados a partir dos ticks) ─────── */
uint32_t timer_get_ticks(void);
uint32_t timer_get_seconds(void);
uint32_t timer_get_uptime_hours(void);
uint32_t timer_get_uptime_minutes(void);
uint32_t timer_get_uptime_seconds(void);

/**
 * @brief Preenche os campos de um "relógio" simples.
 *        Como não temos RTC, começa em 00:00:00 no boot.
 */
void timer_get_clock(uint32_t* hours, uint32_t* minutes, uint32_t* secs);

#endif /* TIMER_H */
