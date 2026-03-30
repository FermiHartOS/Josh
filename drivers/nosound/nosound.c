/*
 *  HEADER FERMI HART OS (Padrão Oficial)
 */
#include <nosound.h>

/* ── Ports ─────────────────────────────────────────────── */
#define SPEAKER_CTRL_PORT 0x61  /* Keyboard Controller (Bit 0 & 1 = Speaker Gate) */
#define PIT_COMMAND_PORT  0x43  /* PIT Command Register */
#define PIT_CH2_PORT      0x42  /* PIT Channel 2 Data */

/* ── Helpers ───────────────────────────────────────────── */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Delay simples baseado em loops (ajustável conforme CPU speed no QEMU) */
static void nosound_delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile int j = 0; j < 50000; j++); 
    }
}

/**
 * @brief Habilita o gate do speaker no Port 0x61.
 */
void speaker_init(void) {
    uint8_t val = inb(SPEAKER_CTRL_PORT);
    /* Bits 0 e 1 devem estar setados para permitir saída do PIT Ch2 no Speaker */
    if ((val & 0x03) != 0x03) {
        outb(SPEAKER_CTRL_PORT, val | 0x03);
    }
}

/**
 * @brief Configura PIT Channel 2 para a frequência desejada.
 *        O clock interno do PIT é 1.193182 MHz.
 */
void speaker_set_freq(uint32_t freq) {
    if (freq == 0) return;

    uint32_t divisor = 1193182 / freq;

    /* Comando: Channel 2, Lobyte/Hibyte, Mode 3 (Square Wave) */
    outb(PIT_COMMAND_PORT, 0xB6); 

    outb(PIT_CH2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH2_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}

/**
 * @brief Liga o som (bit 0 do Port 0x61).
 */
void speaker_on(void) {
    uint8_t val = inb(SPEAKER_CTRL_PORT);
    outb(SPEAKER_CTRL_PORT, val | 0x03);
}

/**
 * @brief Desliga o som (bit 0 do Port 0x61).
 */
void speaker_off(void) {
    uint8_t val = inb(SPEAKER_CTRL_PORT);
    outb(SPEAKER_CTRL_PORT, val & 0xFC); /* Zera bits 0 e 1 */
}

/**
 * @brief Toca um tom por X milissegundos.
 */
void speaker_beep(uint32_t freq, uint32_t duration_ms) {
    speaker_set_freq(freq);
    speaker_on();
    nosound_delay_ms(duration_ms);
    speaker_off();
}
