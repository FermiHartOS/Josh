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
 * @file drivers/keyboard/keyboard.c
 * @brief Keyboard Driver — Shift + CapsLock + Full Scancode Map
 *
 * Features vs. versão anterior:
 *   - Shift press/release tracking (Left Shift 0x2A, Right Shift 0x36)
 *   - CapsLock toggle (scancode 0x3A)
 *   - Maiúsculas: A-Z quando Shift XOR CapsLock
 *   - Shifted numbers: 1→!, 2→@, 3→#, etc.
 *   - Shifted punctuation: ;→:, '→", ,→<, .→>, /→?, -→_, =→+, [→{, ]→}
 *   - Teclas especiais expandidas: F1-F3, Home, End, PgUp, PgDn, Delete
 *   - Key-release agora processa Shift up (não ignora tudo cegamente)
 */

#include <keyboard.h>
#include <stdint.h>

/* ── Buffer circular ──────────────────────────────────────────── */
static char key_buffer[256];
static volatile int head = 0;
static volatile int tail = 0;

/* ── Estado de modificadores ──────────────────────────────────── */
static volatile int shift_held = 0;   /* 1 se qualquer Shift está pressionado */
static volatile int caps_lock  = 0;   /* Toggle: 1 = ativo */

/* ── Scancodes dos modificadores ──────────────────────────────── */
#define SC_LSHIFT_PRESS   0x2A
#define SC_RSHIFT_PRESS   0x36
#define SC_LSHIFT_RELEASE 0xAA  /* 0x2A + 0x80 */
#define SC_RSHIFT_RELEASE 0xB6  /* 0x36 + 0x80 */
#define SC_CAPSLOCK       0x3A

/* ── I/O ──────────────────────────────────────────────────────── */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ── Tabela de caracteres shifted para números ────────────────── */
static char shift_number(char c) {
    switch (c) {
        case '1': return '!'; case '2': return '@'; case '3': return '#';
        case '4': return '$'; case '5': return '%'; case '6': return '^';
        case '7': return '&'; case '8': return '*'; case '9': return '(';
        case '0': return ')';
        default: return c;
    }
}

/* ── Tabela de caracteres shifted para pontuação ──────────────── */
static char shift_punct(char c) {
    switch (c) {
        case '-':  return '_'; case '=':  return '+';
        case '[':  return '{'; case ']':  return '}';
        case ';':  return ':'; case '\'': return '"';
        case ',':  return '<'; case '.':  return '>';
        case '/':  return '?'; case '`':  return '~';
        case '\\': return '|';
        default: return c;
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  IRQ1 Handler — chamado pelo idt.asm
 *
 *  Agora processa key-release para Shift e trata CapsLock como toggle.
 * ══════════════════════════════════════════════════════════════════ */
void keyboard_interrupt_handler(void) {
    if (!(inb(0x64) & 1)) return;

    uint8_t scancode = inb(0x60);

    /* ── Shift press ──────────────────────────────────────── */
    if (scancode == SC_LSHIFT_PRESS || scancode == SC_RSHIFT_PRESS) {
        shift_held = 1;
        return;
    }

    /* ── Shift release ────────────────────────────────────── */
    if (scancode == SC_LSHIFT_RELEASE || scancode == SC_RSHIFT_RELEASE) {
        shift_held = 0;
        return;
    }

    /* ── CapsLock (toggle no press, ignora release) ───────── */
    if (scancode == SC_CAPSLOCK) {
        caps_lock = !caps_lock;
        return;
    }

    /* ── Ignora todas as outras key-releases ──────────────── */
    if (scancode & 0x80) return;

    /* ── Converte scancode para char ──────────────────────── */
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
 *  Aplica Shift e CapsLock:
 *    - Letras: uppercase se (Shift XOR CapsLock)
 *    - Números: shifted symbols se Shift (!, @, #, etc.)
 *    - Pontuação: shifted variants se Shift
 * ══════════════════════════════════════════════════════════════════ */
char keycode_to_char(uint8_t code) {
    char c = 0;
    int is_letter = 0;
    int is_number = 0;
    int is_punct  = 0;

    switch (code) {
        /* Letras */
        case 0x1E: c = 'a'; is_letter = 1; break;
        case 0x30: c = 'b'; is_letter = 1; break;
        case 0x2E: c = 'c'; is_letter = 1; break;
        case 0x20: c = 'd'; is_letter = 1; break;
        case 0x12: c = 'e'; is_letter = 1; break;
        case 0x21: c = 'f'; is_letter = 1; break;
        case 0x22: c = 'g'; is_letter = 1; break;
        case 0x23: c = 'h'; is_letter = 1; break;
        case 0x17: c = 'i'; is_letter = 1; break;
        case 0x24: c = 'j'; is_letter = 1; break;
        case 0x25: c = 'k'; is_letter = 1; break;
        case 0x26: c = 'l'; is_letter = 1; break;
        case 0x32: c = 'm'; is_letter = 1; break;
        case 0x31: c = 'n'; is_letter = 1; break;
        case 0x18: c = 'o'; is_letter = 1; break;
        case 0x19: c = 'p'; is_letter = 1; break;
        case 0x10: c = 'q'; is_letter = 1; break;
        case 0x13: c = 'r'; is_letter = 1; break;
        case 0x1F: c = 's'; is_letter = 1; break;
        case 0x14: c = 't'; is_letter = 1; break;
        case 0x16: c = 'u'; is_letter = 1; break;
        case 0x2F: c = 'v'; is_letter = 1; break;
        case 0x11: c = 'w'; is_letter = 1; break;
        case 0x2D: c = 'x'; is_letter = 1; break;
        case 0x15: c = 'y'; is_letter = 1; break;
        case 0x2C: c = 'z'; is_letter = 1; break;

        /* Números */
        case 0x02: c = '1'; is_number = 1; break;
        case 0x03: c = '2'; is_number = 1; break;
        case 0x04: c = '3'; is_number = 1; break;
        case 0x05: c = '4'; is_number = 1; break;
        case 0x06: c = '5'; is_number = 1; break;
        case 0x07: c = '6'; is_number = 1; break;
        case 0x08: c = '7'; is_number = 1; break;
        case 0x09: c = '8'; is_number = 1; break;
        case 0x0A: c = '9'; is_number = 1; break;
        case 0x0B: c = '0'; is_number = 1; break;

        /* Pontuação */
        case 0x0C: c = '-';  is_punct = 1; break;
        case 0x0D: c = '=';  is_punct = 1; break;
        case 0x1A: c = '[';  is_punct = 1; break;
        case 0x1B: c = ']';  is_punct = 1; break;
        case 0x27: c = ';';  is_punct = 1; break;
        case 0x28: c = '\''; is_punct = 1; break;
        case 0x29: c = '`';  is_punct = 1; break;
        case 0x2B: c = '\\'; is_punct = 1; break;
        case 0x33: c = ',';  is_punct = 1; break;
        case 0x34: c = '.';  is_punct = 1; break;
        case 0x35: c = '/';  is_punct = 1; break;

        /* Controle */
        case 0x39: return ' ';
        case 0x1C: return '\n';
        case 0x0E: return '\b';
        case 0x0F: return '\t';
        case 0x01: return KEY_ESC;

        /* Setas */
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;

        /* Teclas especiais */
        case 0x3B: return KEY_F1;
        case 0x3C: return KEY_F2;
        case 0x3D: return KEY_F3;
        case 0x47: return KEY_HOME;
        case 0x4F: return KEY_END;
        case 0x49: return KEY_PGUP;
        case 0x51: return KEY_PGDN;
        case 0x53: return KEY_DEL;

        default: return 0;
    }

    /* ── Aplica modificadores ──────────────────────────────── */
    if (is_letter) {
        /* Uppercase se (Shift XOR CapsLock) */
        int upper = shift_held ^ caps_lock;
        if (upper) c = c - 32; /* 'a' - 32 = 'A' */
    }
    else if (is_number && shift_held) {
        c = shift_number(c);
    }
    else if (is_punct && shift_held) {
        c = shift_punct(c);
    }

    return c;
}
