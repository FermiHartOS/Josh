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
 * @file user/minibash/minibash.c
 * @brief MiniBash Pro v3 — Shell Reativo do jOSh OS
 *
 * v3 changelog:
 *   - Histórico dinâmico via kmalloc (lista encadeada, sem limite fixo)
 *   - Comando mem expandido: PMM pages + Heap stats + layout
 *   - Comando memtest: self-test automático do kmalloc/kfree
 *   - Comandos malloc/free interativos para debug ao vivo
 *   - Tab completion + Shift/CapsLock support
 */

#include <shell.h>
#include <vga.h>
#include <keyboard.h>
#include <timer.h>
#include <kmalloc.h>
#include <pmm.h>
#include <nosound.h>
#include <stdint.h>

/* ── Identidade do sistema ────────────────────────────────────── */
#define OS_SYSNAME    "jOSh"
#define OS_NODENAME   "fermihart"
#define OS_RELEASE    "1.3.0-reactive"
#define OS_VERSION    "#1 Mon Mar 30 2026"
#define OS_MACHINE    "i686"
#define OS_PROCESSOR  "x86_32"
#define OS_PLATFORM   "bare-metal"

#define SHELL_USER    "fermi"
#define SHELL_CWD     "/"

/* ── Shell config ─────────────────────────────────────────────── */
#define MAX_CMD_LEN   128
#define HISTORY_MAX   32  /* Limite máximo de entradas no histórico dinâmico */

/* ── Estado do shell ──────────────────────────────────────────── */
static char cmd_buffer[MAX_CMD_LEN];
static int  cmd_idx = 0;
static uint32_t cmd_count = 0;

/* ══════════════════════════════════════════════════════════════════
 *  Histórico dinâmico via kmalloc (lista encadeada circular)
 *
 *  Cada entrada é alocada com kmalloc. Quando o histórico atinge
 *  HISTORY_MAX, a entrada mais antiga é liberada com kfree.
 * ══════════════════════════════════════════════════════════════════ */
typedef struct hist_entry {
    char cmd[MAX_CMD_LEN];
    struct hist_entry* next;
    struct hist_entry* prev;
} hist_entry_t;

static hist_entry_t* hist_head    = 0;  /* Mais antigo */
static hist_entry_t* hist_tail    = 0;  /* Mais recente */
static hist_entry_t* hist_browse  = 0;  /* Posição de navegação */
static int           hist_count   = 0;

/* ── Slots de malloc interativo (para o comando malloc/free) ─── */
#define MALLOC_SLOTS 8
static void*    malloc_slots[MALLOC_SLOTS];
static uint32_t malloc_sizes[MALLOC_SLOTS];

/* ══════════════════════════════════════════════════════════════════
 *  Tabela de comandos — Tab Completion
 * ══════════════════════════════════════════════════════════════════ */
static const char* const cmd_table[] = {
    "about", "beep", "clear", "colors", "date", "echo", "env",
    "fetch", "free", "halt", "help", "history", "hostname",
    "malloc", "mem", "memtest", "panic", "play", "reboot",
    "starwars", "uname", "uptime", "ver", "whoami",
};
#define CMD_TABLE_SIZE (sizeof(cmd_table) / sizeof(cmd_table[0]))

/* ══════════════════════════════════════════════════════════════════
 *  String utilities
 * ══════════════════════════════════════════════════════════════════ */
static int str_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static int str_starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return 0;
        str++; prefix++;
    }
    return 1;
}

static int str_len(const char* s) {
    int n = 0; while (s[n]) n++; return n;
}

static void str_copy(char* dest, const char* src) {
    int i = 0; while (src[i]) { dest[i] = src[i]; i++; } dest[i] = '\0';
}

static void itoa(uint32_t val, char* buf) {
    char tmp[12]; int i = 0;
    if (val == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (val > 0) { tmp[i++] = '0' + (val % 10); val /= 10; }
    int j = 0; while (i > 0) buf[j++] = tmp[--i]; buf[j] = '\0';
}

static void itoa_pad2(uint32_t val, char* buf) {
    buf[0] = '0' + (val / 10) % 10;
    buf[1] = '0' + val % 10;
    buf[2] = '\0';
}

static void itoa_hex(uint32_t val, char* buf) {
    const char hex[] = "0123456789ABCDEF";
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 7; i >= 0; i--)
        buf[2 + (7 - i)] = hex[(val >> (i * 4)) & 0xF];
    buf[10] = '\0';
}

static uint32_t parse_uint(const char* s) {
    uint32_t val = 0;
    while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
    return val;
}

/* ══════════════════════════════════════════════════════════════════
 *  Status bar
 * ══════════════════════════════════════════════════════════════════ */
static void draw_status_bar(void) {
    char buf[12];

    for (int x = 0; x < 80; x++)
        vga_put_at(x, 23, 0xC4, COLOR_DARK_GREY);

    vga_fill_row(24, ' ', COLOR_DARK_GREY);

    int pos = 1;

    uint32_t h, m, s;
    timer_get_clock(&h, &m, &s);
    itoa_pad2(h, buf); vga_put_string_at(pos, 24, buf, COLOR_LIGHT_CYAN); pos += 2;
    vga_put_at(pos, 24, ':', COLOR_DARK_GREY); pos++;
    itoa_pad2(m, buf); vga_put_string_at(pos, 24, buf, COLOR_LIGHT_CYAN); pos += 2;
    vga_put_at(pos, 24, ':', COLOR_DARK_GREY); pos++;
    itoa_pad2(s, buf); vga_put_string_at(pos, 24, buf, COLOR_LIGHT_CYAN); pos += 2;

    pos++; vga_put_at(pos, 24, 0xB3, COLOR_DARK_GREY); pos += 2;

    vga_put_string_at(pos, 24, "Up ", COLOR_DARK_GREY); pos += 3;
    itoa(timer_get_uptime_hours(), buf);
    vga_put_string_at(pos, 24, buf, COLOR_CYAN); pos += str_len(buf);
    vga_put_at(pos, 24, 'h', COLOR_DARK_GREY); pos++;
    itoa_pad2(timer_get_uptime_minutes(), buf);
    vga_put_string_at(pos, 24, buf, COLOR_CYAN); pos += 2;
    vga_put_at(pos, 24, 'm', COLOR_DARK_GREY); pos++;

    pos++; vga_put_at(pos, 24, 0xB3, COLOR_DARK_GREY); pos += 2;

    vga_put_string_at(pos, 24, "Cmds:", COLOR_DARK_GREY); pos += 5;
    itoa(cmd_count, buf); vga_put_string_at(pos, 24, buf, COLOR_CYAN); pos += str_len(buf);

    pos++; vga_put_at(pos, 24, 0xB3, COLOR_DARK_GREY); pos += 2;

    vga_put_string_at(pos, 24, "PID:", COLOR_DARK_GREY); pos += 4;
    vga_put_string_at(pos, 24, "0", COLOR_CYAN); pos++;

    pos++; vga_put_at(pos, 24, 0xB3, COLOR_DARK_GREY); pos += 2;

    vga_put_string_at(pos, 24, "Heap:", COLOR_DARK_GREY); pos += 5;
    itoa(heap_get_used(), buf); vga_put_string_at(pos, 24, buf, COLOR_CYAN); pos += str_len(buf);
    vga_put_at(pos, 24, 'B', COLOR_DARK_GREY);

    vga_put_string_at(74, 24, "jOSh", COLOR_LIGHT_CYAN);
    vga_put_at(78, 24, 0xEC, COLOR_LIGHT_MAGENTA);
}

/* ══════════════════════════════════════════════════════════════════
 *  Prompt: fermi@fermihart:/∞ 
 * ══════════════════════════════════════════════════════════════════ */
#define PRINT_PROMPT_INLINE() do { \
    vga_put_string(SHELL_USER, COLOR_LIGHT_GREEN); \
    vga_put_char('@', COLOR_DARK_GREY); \
    vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN); \
    vga_put_char(':', COLOR_DARK_GREY); \
    vga_put_string(SHELL_CWD, COLOR_LIGHT_BLUE); \
    vga_put_char((char)0xEC, COLOR_LIGHT_MAGENTA); \
    vga_put_char(' ', COLOR_WHITE); \
} while(0)

static void print_prompt(void) {
    draw_status_bar();
    vga_put_char('\n', COLOR_WHITE);
    PRINT_PROMPT_INLINE();
}

/* ══════════════════════════════════════════════════════════════════
 *  Line editing
 * ══════════════════════════════════════════════════════════════════ */
static void erase_input_line(void) {
    while (cmd_idx > 0) { vga_put_char('\b', COLOR_WHITE); cmd_idx--; }
}

static void replace_line(const char* new_text) {
    erase_input_line();
    str_copy(cmd_buffer, new_text);
    cmd_idx = str_len(cmd_buffer);
    vga_put_string(cmd_buffer, COLOR_WHITE);
}

/* ══════════════════════════════════════════════════════════════════
 *  Tab Completion
 * ══════════════════════════════════════════════════════════════════ */
static void tab_complete(void) {
    cmd_buffer[cmd_idx] = '\0';
    int match_count = 0, last_match = -1;

    for (int i = 0; i < (int)CMD_TABLE_SIZE; i++) {
        if (cmd_idx == 0 || str_starts_with(cmd_table[i], cmd_buffer)) {
            match_count++; last_match = i;
        }
    }

    if (match_count == 0) return;

    if (match_count == 1) {
        replace_line(cmd_table[last_match]);
        if (cmd_idx < MAX_CMD_LEN - 2) {
            cmd_buffer[cmd_idx++] = ' ';
            cmd_buffer[cmd_idx] = '\0';
            vga_put_char(' ', COLOR_WHITE);
        }
        return;
    }

    vga_put_char('\n', COLOR_WHITE);
    vga_put_string("  ", COLOR_WHITE);
    int col = 0;
    for (int i = 0; i < (int)CMD_TABLE_SIZE; i++) {
        if (cmd_idx == 0 || str_starts_with(cmd_table[i], cmd_buffer)) {
            vga_put_string(cmd_table[i], COLOR_LIGHT_CYAN);
            vga_put_string("  ", COLOR_WHITE);
            if (++col % 6 == 0) vga_put_string("\n  ", COLOR_WHITE);
        }
    }
    vga_put_char('\n', COLOR_WHITE);
    PRINT_PROMPT_INLINE();
    vga_put_string(cmd_buffer, COLOR_WHITE);
}

/* ══════════════════════════════════════════════════════════════════
 *  Histórico dinâmico (kmalloc linked list)
 * ══════════════════════════════════════════════════════════════════ */
static void save_to_history(const char* cmd) {
    if (cmd[0] == '\0') return;

    /* Não salva duplicata do último */
    if (hist_tail && str_compare(hist_tail->cmd, cmd) == 0) return;

    /* Se atingiu o limite, remove o mais antigo */
    if (hist_count >= HISTORY_MAX && hist_head) {
        hist_entry_t* old = hist_head;
        hist_head = hist_head->next;
        if (hist_head) hist_head->prev = 0;
        else hist_tail = 0;
        kfree(old);
        hist_count--;
    }

    /* Aloca nova entrada */
    hist_entry_t* entry = (hist_entry_t*)kmalloc(sizeof(hist_entry_t));
    if (!entry) return; /* Sem memória */

    str_copy(entry->cmd, cmd);
    entry->next = 0;
    entry->prev = hist_tail;

    if (hist_tail) hist_tail->next = entry;
    else hist_head = entry;

    hist_tail = entry;
    hist_count++;
}

static void history_up(void) {
    if (!hist_tail) return;

    if (!hist_browse) {
        hist_browse = hist_tail;
    } else if (hist_browse->prev) {
        hist_browse = hist_browse->prev;
    } else {
        return; /* Já no mais antigo */
    }
    replace_line(hist_browse->cmd);
}

static void history_down(void) {
    if (!hist_browse) return;

    if (hist_browse->next) {
        hist_browse = hist_browse->next;
        replace_line(hist_browse->cmd);
    } else {
        /* Voltou pro mais recente — limpa */
        hist_browse = 0;
        erase_input_line();
        cmd_buffer[0] = '\0';
    }
}

/* ══════════════════════════════════════════════════════════════════
 *  Comandos
 * ══════════════════════════════════════════════════════════════════ */

/* ── uname ─────────────────────────────────────────────────────── */
static void cmd_uname(const char* args) {
    int show_all = 0, show_s = 0, show_n = 0, show_r = 0;
    int show_v = 0, show_m = 0, show_p = 0, show_i = 0;

    if (str_compare(args, "uname") == 0) { show_s = 1; }
    else if (str_starts_with(args, "uname ")) {
        const char* f = args + 6;
        while (*f) {
            if (*f == '-') { f++; continue; }
            if (*f == 'a') show_all = 1;
            if (*f == 's') show_s = 1;
            if (*f == 'n') show_n = 1;
            if (*f == 'r') show_r = 1;
            if (*f == 'v') show_v = 1;
            if (*f == 'm') show_m = 1;
            if (*f == 'p') show_p = 1;
            if (*f == 'i') show_i = 1;
            f++;
        }
    }
    if (show_all) {
        vga_put_string("  ", COLOR_WHITE);
        vga_put_string(OS_SYSNAME, COLOR_LIGHT_CYAN);    vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN);   vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_RELEASE, COLOR_WHITE);           vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_VERSION, COLOR_DARK_GREY);       vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_MACHINE, COLOR_YELLOW);          vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_PROCESSOR, COLOR_YELLOW);        vga_put_char(' ', COLOR_WHITE);
        vga_put_string(OS_PLATFORM, COLOR_DARK_GREY);
        vga_put_char('\n', COLOR_WHITE); return;
    }
    vga_put_string("  ", COLOR_WHITE);
    if (show_s) { vga_put_string(OS_SYSNAME, COLOR_LIGHT_CYAN);  vga_put_char(' ', COLOR_WHITE); }
    if (show_n) { vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN); vga_put_char(' ', COLOR_WHITE); }
    if (show_r) { vga_put_string(OS_RELEASE, COLOR_WHITE);        vga_put_char(' ', COLOR_WHITE); }
    if (show_v) { vga_put_string(OS_VERSION, COLOR_DARK_GREY);    vga_put_char(' ', COLOR_WHITE); }
    if (show_m) { vga_put_string(OS_MACHINE, COLOR_YELLOW);       vga_put_char(' ', COLOR_WHITE); }
    if (show_p) { vga_put_string(OS_PROCESSOR, COLOR_YELLOW);     vga_put_char(' ', COLOR_WHITE); }
    if (show_i) { vga_put_string(OS_PLATFORM, COLOR_DARK_GREY);   vga_put_char(' ', COLOR_WHITE); }
    vga_put_char('\n', COLOR_WHITE);
}

/* ── help ──────────────────────────────────────────────────────── */
static void cmd_help(void) {
    vga_put_string("\n  ", COLOR_WHITE);
    vga_put_string("SYSTEM", COLOR_YELLOW);
    vga_put_string("                 ", COLOR_WHITE);
    vga_put_string("TOOLS", COLOR_YELLOW);
    vga_put_string("                  ", COLOR_WHITE);
    vga_put_string("DANGER\n", COLOR_LIGHT_RED);

    vga_put_string("  help", COLOR_LIGHT_CYAN);     vga_put_string("    this menu    ", COLOR_DARK_GREY);
    vga_put_string("  echo <x>", COLOR_LIGHT_CYAN);  vga_put_string("  print text  ", COLOR_DARK_GREY);
    vga_put_string("  panic", COLOR_LIGHT_RED);       vga_put_string("  kernel panic\n", COLOR_DARK_GREY);

    vga_put_string("  clear", COLOR_LIGHT_CYAN);     vga_put_string("   clear screen  ", COLOR_DARK_GREY);
    vga_put_string("  history", COLOR_LIGHT_CYAN);    vga_put_string("   cmd history", COLOR_DARK_GREY);
    vga_put_string("  halt", COLOR_LIGHT_RED);        vga_put_string("   halt CPU\n", COLOR_DARK_GREY);

    vga_put_string("  fetch", COLOR_LIGHT_CYAN);     vga_put_string("   system info   ", COLOR_DARK_GREY);
    vga_put_string("  colors", COLOR_LIGHT_CYAN);     vga_put_string("    VGA palette", COLOR_DARK_GREY);
    vga_put_string("  reboot", COLOR_LIGHT_RED);      vga_put_string(" reset 8042\n", COLOR_DARK_GREY);

    vga_put_string("  uname -a", COLOR_LIGHT_CYAN);  vga_put_string(" kernel info   ", COLOR_DARK_GREY);
    vga_put_string("  date", COLOR_LIGHT_CYAN);       vga_put_string("      uptime\n", COLOR_DARK_GREY);

    vga_put_string("  whoami", COLOR_LIGHT_CYAN);    vga_put_string("  current user   ", COLOR_DARK_GREY);
    vga_put_string("  env", COLOR_LIGHT_CYAN);        vga_put_string("       variables\n", COLOR_DARK_GREY);

    vga_put_string("  hostname", COLOR_LIGHT_CYAN);  vga_put_string(" machine name  ", COLOR_DARK_GREY);
    vga_put_string("  beep <f> <d>", COLOR_LIGHT_CYAN); vga_put_string(" tone\n", COLOR_DARK_GREY);

    vga_put_string("  mem", COLOR_LIGHT_CYAN);       vga_put_string("      memory map   ", COLOR_DARK_GREY);
    vga_put_string("  play <x>", COLOR_LIGHT_CYAN);   vga_put_string("  melody\n", COLOR_DARK_GREY);

    vga_put_string("  memtest", COLOR_LIGHT_CYAN);   vga_put_string("  heap self-test ", COLOR_DARK_GREY);
    vga_put_string("  starwars", COLOR_LIGHT_CYAN);   vga_put_string("  imperial march\n", COLOR_DARK_GREY);

    vga_put_string("  malloc <n>", COLOR_LIGHT_CYAN); vga_put_string(" alloc bytes  ", COLOR_DARK_GREY);
    vga_put_string("  uptime", COLOR_LIGHT_CYAN);     vga_put_string("  time since boot\n", COLOR_DARK_GREY);

    vga_put_string("  free <slot>", COLOR_LIGHT_CYAN); vga_put_string("  free alloc\n\n", COLOR_DARK_GREY);

    vga_put_string("  Tab=autocomplete  UP/DOWN=history  Shift=uppercase\n", COLOR_DARK_GREY);
}

/* ── fetch ─────────────────────────────────────────────────────── */
static void cmd_fetch(void) {
    char buf[12];
    vga_put_char('\n', COLOR_WHITE);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string(OS_SYSNAME, COLOR_WHITE); vga_put_string(" ", COLOR_WHITE);
    vga_put_string(OS_RELEASE, COLOR_DARK_GREY); vga_put_char('\n', COLOR_WHITE);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("Kernel: ", COLOR_DARK_GREY);
    vga_put_string(OS_MACHINE, COLOR_WHITE);
    vga_put_string(" Monolithic Reactive\n", COLOR_WHITE);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("Shell: MiniBash Pro v3\n", COLOR_WHITE);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("Host: ", COLOR_DARK_GREY);
    vga_put_string(OS_NODENAME, COLOR_WHITE);
    vga_put_string(" (", COLOR_DARK_GREY); vga_put_string(OS_PLATFORM, COLOR_WHITE);
    vga_put_string(")\n", COLOR_DARK_GREY);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("Uptime: ", COLOR_DARK_GREY);
    itoa(timer_get_uptime_hours(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("h ", COLOR_DARK_GREY);
    itoa(timer_get_uptime_minutes(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("m ", COLOR_DARK_GREY);
    itoa(timer_get_uptime_seconds(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("s\n", COLOR_DARK_GREY);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("RAM: ", COLOR_DARK_GREY);
    itoa(pmm_get_free_kb(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("KB free / ", COLOR_DARK_GREY);
    itoa(pmm_get_total_kb(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("KB total\n", COLOR_DARK_GREY);
    vga_put_string("  ######   ", COLOR_LIGHT_CYAN);
    vga_put_string("Heap: ", COLOR_DARK_GREY);
    itoa(heap_get_used(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("B used / ", COLOR_DARK_GREY);
    itoa(heap_get_total(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("B total (", COLOR_DARK_GREY);
    itoa(heap_get_alloc_count(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" allocs)\n", COLOR_DARK_GREY);
}

/* ── mem (expandido) ───────────────────────────────────────────── */
static void cmd_mem(void) {
    char buf[12];

    vga_put_string("  Physical Memory (PMM):\n", COLOR_YELLOW);
    vga_put_string("    Total:  ", COLOR_WHITE);
    itoa(pmm_get_total_kb(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" KB (", COLOR_DARK_GREY);
    itoa(pmm_get_total_pages(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" pages x 4KB)\n", COLOR_DARK_GREY);
    vga_put_string("    Used:   ", COLOR_WHITE);
    itoa(pmm_get_used_pages() * 4, buf); vga_put_string(buf, COLOR_LIGHT_RED);
    vga_put_string(" KB (", COLOR_DARK_GREY);
    itoa(pmm_get_used_pages(), buf); vga_put_string(buf, COLOR_LIGHT_RED);
    vga_put_string(" pages)\n", COLOR_DARK_GREY);
    vga_put_string("    Free:   ", COLOR_WHITE);
    itoa(pmm_get_free_kb(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" KB (", COLOR_DARK_GREY);
    itoa(pmm_get_free_pages(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" pages)\n", COLOR_DARK_GREY);

    vga_put_string("  Kernel Heap (kmalloc):\n", COLOR_YELLOW);
    vga_put_string("    Total:  ", COLOR_WHITE);
    itoa(heap_get_total(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" bytes\n", COLOR_DARK_GREY);
    vga_put_string("    Used:   ", COLOR_WHITE);
    itoa(heap_get_used(), buf); vga_put_string(buf, COLOR_LIGHT_RED);
    vga_put_string(" bytes (", COLOR_DARK_GREY);
    itoa(heap_get_alloc_count(), buf); vga_put_string(buf, COLOR_LIGHT_RED);
    vga_put_string(" allocations)\n", COLOR_DARK_GREY);
    vga_put_string("    Free:   ", COLOR_WHITE);
    itoa(heap_get_free(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" bytes\n", COLOR_DARK_GREY);

    vga_put_string("  Layout:\n", COLOR_YELLOW);
    vga_put_string("    VGA:    ", COLOR_WHITE); itoa_hex(0xB8000, buf);
    vga_put_string(buf, COLOR_LIGHT_GREEN); vga_put_string("  (4000B)\n", COLOR_DARK_GREY);
    vga_put_string("    Kernel: ", COLOR_WHITE); itoa_hex(0x100000, buf);
    vga_put_string(buf, COLOR_LIGHT_GREEN); vga_put_string("  (1MB mark)\n", COLOR_DARK_GREY);
    vga_put_string("    ESP:    ", COLOR_WHITE);
    uint32_t esp; asm volatile("mov %%esp, %0" : "=r"(esp));
    itoa_hex(esp, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_char('\n', COLOR_WHITE);
}

/* ── memtest (self-test do kmalloc/kfree) ──────────────────────── */
static void cmd_memtest(void) {
    char buf[12];
    vga_put_string("  Heap Self-Test:\n", COLOR_YELLOW);

    uint32_t used_before = heap_get_used();
    uint32_t alloc_before = heap_get_alloc_count();
    int pass = 1;

    /* Teste 1: Alocar 8 blocos de tamanhos variados */
    vga_put_string("    Alloc 8 blocks... ", COLOR_WHITE);
    void* ptrs[8];
    uint32_t sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048};
    for (int i = 0; i < 8; i++) {
        ptrs[i] = kmalloc(sizes[i]);
        if (!ptrs[i]) { pass = 0; break; }
    }
    if (pass) {
        vga_put_string("[PASS]\n", COLOR_LIGHT_GREEN);
    } else {
        vga_put_string("[FAIL] kmalloc returned NULL\n", COLOR_LIGHT_RED);
        return;
    }

    /* Teste 2: Verificar que ponteiros são distintos */
    vga_put_string("    Check distinct... ", COLOR_WHITE);
    for (int i = 0; i < 8 && pass; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (ptrs[i] == ptrs[j]) { pass = 0; break; }
        }
    }
    vga_put_string(pass ? "[PASS]\n" : "[FAIL] duplicate ptrs\n",
                   pass ? COLOR_LIGHT_GREEN : COLOR_LIGHT_RED);

    /* Teste 3: Escrever padrão e ler de volta */
    vga_put_string("    Write/read test.. ", COLOR_WHITE);
    for (int i = 0; i < 8 && pass; i++) {
        uint8_t* p = (uint8_t*)ptrs[i];
        uint8_t pattern = (uint8_t)(0xA5 ^ i);
        for (uint32_t b = 0; b < sizes[i]; b++) p[b] = pattern;
        for (uint32_t b = 0; b < sizes[i]; b++) {
            if (p[b] != pattern) { pass = 0; break; }
        }
    }
    vga_put_string(pass ? "[PASS]\n" : "[FAIL] data corruption\n",
                   pass ? COLOR_LIGHT_GREEN : COLOR_LIGHT_RED);

    /* Teste 4: Heap used deve ter crescido */
    vga_put_string("    Heap grew......  ", COLOR_WHITE);
    uint32_t used_mid = heap_get_used();
    if (used_mid > used_before) {
        vga_put_string("[PASS] +", COLOR_LIGHT_GREEN);
        itoa(used_mid - used_before, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
        vga_put_string("B\n", COLOR_LIGHT_GREEN);
    } else {
        vga_put_string("[FAIL]\n", COLOR_LIGHT_RED); pass = 0;
    }

    /* Teste 5: Free todos */
    vga_put_string("    Free all blocks. ", COLOR_WHITE);
    for (int i = 0; i < 8; i++) kfree(ptrs[i]);
    uint32_t used_after = heap_get_used();
    uint32_t alloc_after = heap_get_alloc_count();
    if (used_after == used_before && alloc_after == alloc_before) {
        vga_put_string("[PASS] heap restored\n", COLOR_LIGHT_GREEN);
    } else {
        vga_put_string("[WARN] ", COLOR_YELLOW);
        itoa(used_after, buf); vga_put_string(buf, COLOR_YELLOW);
        vga_put_string("B used (was ", COLOR_YELLOW);
        itoa(used_before, buf); vga_put_string(buf, COLOR_YELLOW);
        vga_put_string("B)\n", COLOR_YELLOW);
    }

    /* Resultado */
    vga_put_string("  Result: ", COLOR_WHITE);
    if (pass) {
        vga_put_string("ALL TESTS PASSED", COLOR_LIGHT_GREEN);
    } else {
        vga_put_string("SOME TESTS FAILED", COLOR_LIGHT_RED);
    }
    vga_put_char('\n', COLOR_WHITE);
}

/* ── malloc interativo ─────────────────────────────────────────── */
static void cmd_malloc_interactive(const char* args) {
    char buf[12];
    const char* p = args + 7; /* Pula "malloc " */
    while (*p == ' ') p++;
    uint32_t size = parse_uint(p);

    if (size == 0) {
        vga_put_string("  Uso: malloc <bytes>\n", COLOR_LIGHT_RED);
        return;
    }

    /* Procura slot livre */
    int slot = -1;
    for (int i = 0; i < MALLOC_SLOTS; i++) {
        if (!malloc_slots[i]) { slot = i; break; }
    }
    if (slot == -1) {
        vga_put_string("  Todos os slots ocupados. Use free <n> primeiro.\n", COLOR_LIGHT_RED);
        return;
    }

    void* ptr = kmalloc(size);
    if (!ptr) {
        vga_put_string("  kmalloc falhou (sem memoria)\n", COLOR_LIGHT_RED);
        return;
    }

    malloc_slots[slot] = ptr;
    malloc_sizes[slot] = size;

    vga_put_string("  Slot ", COLOR_WHITE);
    itoa(slot, buf); vga_put_string(buf, COLOR_LIGHT_CYAN);
    vga_put_string(": ", COLOR_WHITE);
    itoa(size, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("B @ ", COLOR_WHITE);
    itoa_hex((uint32_t)ptr, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("  (heap used: ", COLOR_DARK_GREY);
    itoa(heap_get_used(), buf); vga_put_string(buf, COLOR_CYAN);
    vga_put_string("B)\n", COLOR_DARK_GREY);
}

/* ── free interativo ───────────────────────────────────────────── */
static void cmd_free_interactive(const char* args) {
    char buf[12];

    /* "free" sem argumento: mostra slots */
    if (str_compare(args, "free") == 0) {
        vga_put_string("  Allocation slots:\n", COLOR_YELLOW);
        int any = 0;
        for (int i = 0; i < MALLOC_SLOTS; i++) {
            if (malloc_slots[i]) {
                any = 1;
                vga_put_string("    [", COLOR_WHITE);
                itoa(i, buf); vga_put_string(buf, COLOR_LIGHT_CYAN);
                vga_put_string("] ", COLOR_WHITE);
                itoa(malloc_sizes[i], buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
                vga_put_string("B @ ", COLOR_WHITE);
                itoa_hex((uint32_t)malloc_slots[i], buf);
                vga_put_string(buf, COLOR_LIGHT_GREEN);
                vga_put_char('\n', COLOR_WHITE);
            }
        }
        if (!any) vga_put_string("    (nenhuma alocacao ativa)\n", COLOR_DARK_GREY);
        return;
    }

    /* "free <n>" — libera o slot */
    const char* p = args + 5; /* Pula "free " */
    while (*p == ' ') p++;
    uint32_t slot = parse_uint(p);

    if (slot >= MALLOC_SLOTS || !malloc_slots[slot]) {
        vga_put_string("  Slot invalido ou vazio\n", COLOR_LIGHT_RED);
        return;
    }

    kfree(malloc_slots[slot]);
    vga_put_string("  Freed slot ", COLOR_WHITE);
    itoa(slot, buf); vga_put_string(buf, COLOR_LIGHT_CYAN);
    vga_put_string(" (", COLOR_DARK_GREY);
    itoa(malloc_sizes[slot], buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("B)  heap used: ", COLOR_DARK_GREY);
    malloc_slots[slot] = 0;
    malloc_sizes[slot] = 0;
    itoa(heap_get_used(), buf); vga_put_string(buf, COLOR_CYAN);
    vga_put_string("B\n", COLOR_DARK_GREY);
}

/* ── outros comandos ───────────────────────────────────────────── */
static void cmd_echo(const char* args) {
    vga_put_string("  ", COLOR_WHITE); vga_put_string(args + 5, COLOR_WHITE);
    vga_put_char('\n', COLOR_WHITE);
}

static void cmd_date(void) {
    char buf[4]; uint32_t h, m, s; timer_get_clock(&h, &m, &s);
    vga_put_string("  ", COLOR_WHITE);
    itoa_pad2(h, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_char(':', COLOR_DARK_GREY);
    itoa_pad2(m, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_char(':', COLOR_DARK_GREY);
    itoa_pad2(s, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("  (uptime since boot)\n", COLOR_DARK_GREY);
}

static void cmd_uptime_display(void) {
    char buf[12];
    vga_put_string("  up ", COLOR_WHITE);
    itoa(timer_get_uptime_hours(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("h ", COLOR_DARK_GREY);
    itoa(timer_get_uptime_minutes(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("m ", COLOR_DARK_GREY);
    itoa(timer_get_uptime_seconds(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("s  (", COLOR_DARK_GREY);
    itoa(timer_get_ticks(), buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string(" ticks)\n", COLOR_DARK_GREY);
}

static void cmd_env(void) {
    vga_put_string("  USER=", COLOR_DARK_GREY); vga_put_string(SHELL_USER, COLOR_LIGHT_GREEN);
    vga_put_string("\n  HOSTNAME=", COLOR_DARK_GREY); vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN);
    vga_put_string("\n  PWD=", COLOR_DARK_GREY); vga_put_string(SHELL_CWD, COLOR_LIGHT_GREEN);
    vga_put_string("\n  SHELL=/bin/minibash\n  TERM=vga80x25", COLOR_DARK_GREY);
    vga_put_string("\n  ARCH=", COLOR_DARK_GREY); vga_put_string(OS_MACHINE, COLOR_LIGHT_GREEN);
    vga_put_string("\n  SYSNAME=", COLOR_DARK_GREY); vga_put_string(OS_SYSNAME, COLOR_LIGHT_GREEN);
    vga_put_string("\n  RELEASE=", COLOR_DARK_GREY); vga_put_string(OS_RELEASE, COLOR_LIGHT_GREEN);
    vga_put_string("\n  TIMER_HZ=100\n", COLOR_DARK_GREY);
}

static void cmd_colors(void) {
    vga_put_string("  VGA 16-color palette:\n  ", COLOR_WHITE);
    for (uint8_t i = 0; i < 16; i++) {
        vga_put_char((char)0xDB, i); vga_put_char((char)0xDB, i);
        vga_put_char(' ', COLOR_WHITE);
    }
    vga_put_char('\n', COLOR_WHITE);
}

static void cmd_history_list(void) {
    if (hist_count == 0) {
        vga_put_string("  (empty history)\n", COLOR_DARK_GREY); return;
    }
    char buf[4]; int n = 1;
    hist_entry_t* e = hist_head;
    while (e) {
        vga_put_string("  ", COLOR_WHITE);
        itoa(n++, buf); vga_put_string(buf, COLOR_DARK_GREY);
        vga_put_string("  ", COLOR_WHITE);
        vga_put_string(e->cmd, COLOR_LIGHT_GREY);
        vga_put_char('\n', COLOR_WHITE);
        e = e->next;
    }
}

static void cmd_about(void) {
    vga_put_char('\n', COLOR_WHITE);
    vga_put_string("  jOSh OS", COLOR_LIGHT_CYAN);
    vga_put_string(" - by ", COLOR_DARK_GREY);
    vga_put_string("Fermi Hart\n", COLOR_WHITE);
    vga_put_string("  A bare-metal x86 operating system built from scratch.\n", COLOR_WHITE);
    vga_put_string("  No libc. No stdlib. No safety net.\n", COLOR_DARK_GREY);
    vga_put_string("  Dedicated to Josh.\n\n", COLOR_LIGHT_MAGENTA);
    vga_put_string("  github.com/fermihart | contact@fermihart.com\n", COLOR_LIGHT_BLUE);
}

static void cmd_reboot(void) {
    vga_put_string("  Rebooting via 8042...\n", COLOR_YELLOW);
    uint8_t tmp;
    do { asm volatile("inb $0x64, %0" : "=a"(tmp)); } while (tmp & 0x02);
    asm volatile("outb %0, $0x64" : : "a"((uint8_t)0xFE));
    asm volatile("cli; hlt");
}

/* ── Audio ─────────────────────────────────────────────────────── */
static void cmd_beep(const char* args) {
    char buf[12];
    const char* p = args + 5;
    while (*p == ' ') p++;
    uint32_t freq = parse_uint(p);
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ') p++;
    uint32_t dur = parse_uint(p);

    if (freq == 0 || dur == 0) {
        vga_put_string("  Uso: beep <freq_hz> <dur_ms>\n", COLOR_LIGHT_RED);
        return;
    }
    vga_put_string("  ", COLOR_WHITE);
    itoa(freq, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("Hz x ", COLOR_WHITE);
    itoa(dur, buf); vga_put_string(buf, COLOR_LIGHT_GREEN);
    vga_put_string("ms\n", COLOR_WHITE);
    speaker_beep(freq, dur);
}

static void cmd_play(const char* args) {
    const char* name = args + 5;
    while (*name == ' ') name++;
    if (str_compare(name, "tetris") == 0) {
        vga_put_string("  Playing Tetris Theme...\n", COLOR_YELLOW);
        speaker_beep(659,100); speaker_beep(659,100); speaker_beep(659,100);
        speaker_beep(523,100); speaker_beep(523,100); speaker_beep(523,100);
        speaker_beep(493,100); speaker_beep(493,100); speaker_beep(493,100);
        speaker_beep(587,100); speaker_beep(587,100); speaker_beep(587,100);
        speaker_beep(659,100); speaker_beep(659,100); speaker_beep(659,100);
        speaker_beep(698,200);
        vga_put_string("  Done!\n", COLOR_GREEN);
    } else if (str_compare(name, "startup") == 0) {
        vga_put_string("  Power On Sound...\n", COLOR_CYAN);
        speaker_beep(220,200); speaker_beep(330,200); speaker_beep(440,400);
        vga_put_string("  Ready.\n", COLOR_GREEN);
    } else {
        vga_put_string("  Unknown melody. Try: tetris, startup\n", COLOR_LIGHT_RED);
    }
}

static void cmd_starwars(void) {
    vga_put_string("  Playing Imperial March (Soft)...\n", COLOR_YELLOW);
    speaker_beep_soft(165,150); speaker_beep_soft(165,150); speaker_beep_soft(165,150);
    nosound_delay_ms(100);
    speaker_beep_soft(208,150); speaker_beep_soft(208,150); speaker_beep_soft(208,150);
    nosound_delay_ms(100);
    speaker_beep_soft(262,150); speaker_beep_soft(262,150); speaker_beep_soft(262,150);
    nosound_delay_ms(100);
    speaker_beep_soft(247,150); speaker_beep_soft(247,150); speaker_beep_soft(247,150);
    nosound_delay_ms(100);
    speaker_beep_soft(220,150); speaker_beep_soft(220,150); speaker_beep_soft(208,300);
    nosound_delay_ms(200);
    speaker_beep_soft(262,150); speaker_beep_soft(262,150); speaker_beep_soft(262,150);
    nosound_delay_ms(100);
    speaker_beep_soft(294,150); speaker_beep_soft(294,150); speaker_beep_soft(294,150);
    nosound_delay_ms(100);
    speaker_beep_soft(330,150); speaker_beep_soft(330,150); speaker_beep_soft(330,150);
    nosound_delay_ms(100);
    speaker_beep_soft(370,150); speaker_beep_soft(370,150); speaker_beep_soft(370,150);
    nosound_delay_ms(100);
    speaker_beep_soft(330,150); speaker_beep_soft(330,150); speaker_beep_soft(294,300);
    vga_put_string("  Done!\n", COLOR_GREEN);
}

/* ══════════════════════════════════════════════════════════════════
 *  Command Dispatcher
 * ══════════════════════════════════════════════════════════════════ */
static void execute_command(void) {
    cmd_buffer[cmd_idx] = '\0';

    /* Trim trailing spaces */
    while (cmd_idx > 0 && cmd_buffer[cmd_idx - 1] == ' ') {
        cmd_idx--; cmd_buffer[cmd_idx] = '\0';
    }

    save_to_history(cmd_buffer);
    hist_browse = 0;
    cmd_count++;

    vga_put_char('\n', COLOR_WHITE);

    if (cmd_idx == 0) { /* empty */ }
    else if (str_compare(cmd_buffer, "help") == 0) cmd_help();
    else if (str_compare(cmd_buffer, "clear") == 0) { vga_clear(); draw_status_bar(); }
    else if (str_compare(cmd_buffer, "fetch") == 0) cmd_fetch();
    else if (str_starts_with(cmd_buffer, "uname")) cmd_uname(cmd_buffer);
    else if (str_starts_with(cmd_buffer, "beep ")) cmd_beep(cmd_buffer);
    else if (str_compare(cmd_buffer, "beep") == 0) {
        vga_put_string("  Uso: beep <freq_hz> <dur_ms>\n", COLOR_LIGHT_RED);
    }
    else if (str_starts_with(cmd_buffer, "play ")) cmd_play(cmd_buffer);
    else if (str_compare(cmd_buffer, "play") == 0) {
        vga_put_string("  Uso: play <melody>\n", COLOR_LIGHT_RED);
    }
    else if (str_compare(cmd_buffer, "starwars") == 0) cmd_starwars();
    else if (str_starts_with(cmd_buffer, "malloc ")) cmd_malloc_interactive(cmd_buffer);
    else if (str_starts_with(cmd_buffer, "free ")) cmd_free_interactive(cmd_buffer);
    else if (str_compare(cmd_buffer, "free") == 0) cmd_free_interactive(cmd_buffer);
    else if (str_compare(cmd_buffer, "whoami") == 0) {
        vga_put_string("  ", COLOR_WHITE); vga_put_string(SHELL_USER, COLOR_LIGHT_GREEN);
        vga_put_char('\n', COLOR_WHITE);
    }
    else if (str_compare(cmd_buffer, "hostname") == 0) {
        vga_put_string("  ", COLOR_WHITE); vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN);
        vga_put_char('\n', COLOR_WHITE);
    }
    else if (str_compare(cmd_buffer, "date") == 0) cmd_date();
    else if (str_compare(cmd_buffer, "uptime") == 0) cmd_uptime_display();
    else if (str_compare(cmd_buffer, "mem") == 0) cmd_mem();
    else if (str_compare(cmd_buffer, "memtest") == 0) cmd_memtest();
    else if (str_compare(cmd_buffer, "env") == 0) cmd_env();
    else if (str_compare(cmd_buffer, "colors") == 0) cmd_colors();
    else if (str_compare(cmd_buffer, "history") == 0) cmd_history_list();
    else if (str_compare(cmd_buffer, "about") == 0) cmd_about();
    else if (str_starts_with(cmd_buffer, "echo ")) cmd_echo(cmd_buffer);
    else if (str_compare(cmd_buffer, "ver") == 0) {
        vga_put_string("  ", COLOR_WHITE); vga_put_string(OS_SYSNAME, COLOR_LIGHT_CYAN);
        vga_put_string(" [", COLOR_WHITE); vga_put_string(OS_RELEASE, COLOR_WHITE);
        vga_put_string("]\n", COLOR_WHITE);
    }
    else if (str_compare(cmd_buffer, "panic") == 0) {
        vga_clear();
        vga_put_string("KERNEL PANIC: User triggered panic test.\n", COLOR_LIGHT_RED);
        vga_put_string("System halted. Reboot required.", COLOR_WHITE);
        asm volatile("cli; hlt");
    }
    else if (str_compare(cmd_buffer, "halt") == 0) {
        vga_put_string("  Powering down...\n", COLOR_WHITE);
        asm volatile("cli; hlt");
    }
    else if (str_compare(cmd_buffer, "reboot") == 0) cmd_reboot();
    else {
        vga_put_string("  minibash: ", COLOR_WHITE);
        vga_put_string(cmd_buffer, COLOR_LIGHT_RED);
        vga_put_string(": command not found\n", COLOR_WHITE);
    }

    for (int i = 0; i < MAX_CMD_LEN; i++) cmd_buffer[i] = 0;
    cmd_idx = 0;
    print_prompt();
}

/* ══════════════════════════════════════════════════════════════════
 *  Shell entry point
 * ══════════════════════════════════════════════════════════════════ */
void start_shell(void) {
    /* Inicializa slots de malloc interativo */
    for (int i = 0; i < MALLOC_SLOTS; i++) {
        malloc_slots[i] = 0;
        malloc_sizes[i] = 0;
    }

    vga_clear();
    draw_status_bar();

    vga_put_string("Welcome to ", COLOR_WHITE);
    vga_put_string("jOSh OS", COLOR_LIGHT_CYAN);
    vga_put_string(" (", COLOR_DARK_GREY);
    vga_put_string(OS_NODENAME, COLOR_LIGHT_GREEN);
    vga_put_string(")\n", COLOR_DARK_GREY);
    vga_put_string("Type 'help' for available commands.\n", COLOR_DARK_GREY);

    print_prompt();

    while (1) {
        char c = get_key_from_buffer();
        if (c == 0) { asm volatile("hlt"); continue; }

        draw_status_bar();

        if (c == KEY_UP)   { history_up();   continue; }
        if (c == KEY_DOWN) { history_down(); continue; }
        if (c == '\n')     { execute_command(); continue; }
        if (c == '\b') {
            if (cmd_idx > 0) { cmd_idx--; cmd_buffer[cmd_idx] = '\0'; vga_put_char('\b', COLOR_WHITE); }
            continue;
        }
        if (c == '\t') { tab_complete(); continue; }
        if (cmd_idx < MAX_CMD_LEN - 1) {
            cmd_buffer[cmd_idx++] = c;
            vga_put_char(c, COLOR_WHITE);
        }
    }
}
