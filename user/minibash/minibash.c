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
#include <shell.h>
#include <vga.h>
#include <keyboard.h>

#define MAX_CMD_LEN 128
static char cmd_buffer[MAX_CMD_LEN];
static int cmd_idx = 0;

int str_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void print_prompt() {
    vga_put_string("\nFermi@root: ", COLOR_LIGHT_CYAN);
}

void cmd_fetch() {
    vga_put_string("\n  ######   Fermi Hart OS v1.2", COLOR_LIGHT_CYAN);
    vga_put_string("\n  ######   Kernel: x86_32 Monolithic", COLOR_WHITE);
    vga_put_string("\n  ######   Shell: MiniBash Pro", COLOR_WHITE);
    vga_put_string("\n  ######   Uptime: Just booted", COLOR_WHITE);
    vga_put_char('\n', COLOR_WHITE);
}

void execute_command() {
    cmd_buffer[cmd_idx] = '\0';
    vga_put_char('\n', COLOR_WHITE);

    if (str_compare(cmd_buffer, "help") == 0) {
        vga_put_string("COMMANDS: help, clear, fetch, ver, color, panic, halt", COLOR_YELLOW);
    } 
    else if (str_compare(cmd_buffer, "clear") == 0) {
        vga_clear();
    } 
    else if (str_compare(cmd_buffer, "fetch") == 0) {
        cmd_fetch();
    }
    else if (str_compare(cmd_buffer, "ver") == 0) {
        vga_put_string("Fermi Hart OS [Version 1.2.54-Pro]", COLOR_WHITE);
    }
    else if (str_compare(cmd_buffer, "panic") == 0) {
        vga_clear();
        vga_put_string("KERNEL PANIC: User triggered panic test.", COLOR_LIGHT_RED);
        asm volatile("cli; hlt");
    }
    else if (str_compare(cmd_buffer, "halt") == 0) {
        vga_put_string("Powering down...", COLOR_WHITE);
        asm volatile("hlt");
    }
    else if (cmd_idx > 0) {
        vga_put_string("MiniBash: '", COLOR_WHITE);
        vga_put_string(cmd_buffer, COLOR_LIGHT_RED);
        vga_put_string("' not found.", COLOR_WHITE);
    }

    cmd_idx = 0;
    for(int i=0; i<MAX_CMD_LEN; i++) cmd_buffer[i] = 0;
    print_prompt();
}

void start_shell() {
    vga_clear();
    vga_put_string("Welcome to ", COLOR_WHITE);
    vga_put_string("Fermi Hart OS Pro\n", COLOR_LIGHT_CYAN);
    vga_put_string("Type 'help' for available tools.\n", COLOR_DARK_GREY);
    print_prompt();
    
    while(1) {
        uint8_t code = get_keycode();
        if (code & 0x80) continue; 

        char c = keycode_to_char(code);
        if (c == '\n') {
            execute_command();
        } else if (c == '\b') {
            if (cmd_idx > 0) {
                cmd_idx--;
                vga_put_char('\b', COLOR_WHITE);
            }
        } else if (c != 0 && cmd_idx < MAX_CMD_LEN - 1) {
            cmd_buffer[cmd_idx++] = c;
            vga_put_char(c, COLOR_WHITE);
        }
    }
}
