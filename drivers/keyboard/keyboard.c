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

/*
 * drivers/keyboard/keyboard.c - Driver Reativo com Suporte a Setas e Números
 */
#include <keyboard.h>
#include <stdint.h>

/* ── Buffer circular ──────────────────────────────────────────── */
static char key_buffer[256];
static volatile int head = 0;
static volatile int tail = 0;

/* ── I/O ──────────────────────────────────────────────────────── */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ── IRQ1 Handler (chamado pelo idt.asm) ──────────────────────── */
void keyboard_interrupt_handler(void) {
    if (!(inb(0x64) & 1)) return;

    uint8_t scancode = inb(0x60);

    /* Ignora key-release (bit 7 set) */
    if (scancode & 0x80) return;

    char c = keycode_to_char(scancode);

    if (c != 0) {
        key_buffer[head] = c;
        head = (head + 1) % 256;
    }
}

/* ── Leitura do buffer (usada pelo shell) ─────────────────────── */
char get_key_from_buffer(void) {
    if (head == tail) return 0;
    char c = key_buffer[tail];
    tail = (tail + 1) % 256;
    return c;
}

/* ══════════════════════════════════════════════════════════════════
 *  Mapa de conversão scancode → char
 *
 *  Adicionado vs. versão anterior:
 *    - Números 0-9
 *    - Setas UP (0x48) → KEY_UP, DOWN (0x50) → KEY_DOWN
 *    - Tab (0x0F) → '\t'
 *    - Pontuação básica: . , - = / ; ' [ ]
 *    - ESC (0x01) → '\x1A'
 * ══════════════════════════════════════════════════════════════════ */
char keycode_to_char(uint8_t code) {
    switch (code) {
        /* Letras */
        case 0x1E: return 'a'; case 0x30: return 'b'; case 0x2E: return 'c';
        case 0x20: return 'd'; case 0x12: return 'e'; case 0x21: return 'f';
        case 0x22: return 'g'; case 0x23: return 'h'; case 0x17: return 'i';
        case 0x24: return 'j'; case 0x25: return 'k'; case 0x26: return 'l';
        case 0x32: return 'm'; case 0x31: return 'n'; case 0x18: return 'o';
        case 0x19: return 'p'; case 0x10: return 'q'; case 0x13: return 'r';
        case 0x1F: return 's'; case 0x14: return 't'; case 0x16: return 'u';
        case 0x2F: return 'v'; case 0x11: return 'w'; case 0x2D: return 'x';
        case 0x15: return 'y'; case 0x2C: return 'z';

        /* Números */
        case 0x0B: return '0'; case 0x02: return '1'; case 0x03: return '2';
        case 0x04: return '3'; case 0x05: return '4'; case 0x06: return '5';
        case 0x07: return '6'; case 0x08: return '7'; case 0x09: return '8';
        case 0x0A: return '9';

        /* Pontuação básica */
        case 0x33: return ','; case 0x34: return '.'; case 0x0C: return '-';
        case 0x0D: return '='; case 0x35: return '/'; case 0x27: return ';';
        case 0x28: return '\''; case 0x1A: return '['; case 0x1B: return ']';

        /* Controle */
        case 0x39: return ' ';     /* Espaço */
        case 0x1C: return '\n';    /* Enter  */
        case 0x0E: return '\b';    /* Backspace */
        case 0x0F: return '\t';    /* Tab */

        /* Setas — mapeadas para chars especiais definidos em keyboard.h */
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;

        default: return 0;
    }
}
