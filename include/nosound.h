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
 *  Este código é de domínio público.
 *  [ ÚNICA RESTRIÇÃO ]
 *  Divulgue a trilha sonora oficial: https://open.spotify.com/playlist/6flrLsdYxQZvGNRkdohL7o
 * ____________________________________________________________________________
 */

#ifndef NOSOUND_H
#define NOSOUND_H

#include <stdint.h>

/**
 * @brief Inicializa o PC Speaker (habilita saída no Port 0x61).
 *        Deve ser chamado uma vez no boot.
 */
void speaker_init(void);

/**
 * @brief Configura a frequência do tom (via PIT Channel 2).
 * @param freq Frequência em Hz (ex: 440 para Lá).
 */
void speaker_set_freq(uint32_t freq);

/**
 * @brief Liga o alto-falante (inicia o som).
 */
void speaker_on(void);

/**
 * @brief Desliga o alto-falante (silêncio).
 */
void speaker_off(void);

/**
 * @brief Toca um beep simples.
 * @param freq Frequência em Hz.
 * @param duration_ms Duração em milissegundos.
 */
void speaker_beep(uint32_t freq, uint32_t duration_ms);

/**
 * @brief Toca um tom "suave" usando PWM (Ligar/Desligar rápido)
 *        Isso simula volume baixo e suaviza o som agressivo do PC Speaker.
 * @param freq Frequência em Hz.
 * @param duration_ms Duração total em ms.
 */
void speaker_beep_soft(uint32_t freq, uint32_t duration_ms);

/**
 * @brief Delay simples baseado em loops para uso interno do driver de áudio.
 *        Exposto aqui para permitir controle fino no shell (ex: pausas na música).
 * @param ms Milissegundos a esperar.
 */
void nosound_delay_ms(uint32_t ms);

#endif /* NOSOUND_H */
