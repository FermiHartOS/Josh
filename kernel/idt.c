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
 *  AUTHOR: F E R M I  ∞  H A R T <contact@fermihart.com>
 *  PROJECT: jOSh - Operating System
 */
/**
 * @file kernel/idt.c
 * @brief IDT + PIC remapping + PIT timer init
 */

#include <idt.h>
#include <vga.h>
#include <keyboard.h>
#include <timer.h>

static idt_entry_t idt[256];
static idt_ptr_t   idtp;

extern void irq_handler_keyboard(void);
extern void irq_handler_timer(void);
extern void exception_div_zero(void);
extern void idt_load(uint32_t);

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait(void) {
    asm volatile("outb %%al, $0x80" : : "a"(0));
}

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low    = base & 0xFFFF;
    idt[num].selector    = sel;
    idt[num].always_zero = 0;
    idt[num].flags       = flags;
    idt[num].base_high   = (base >> 16) & 0xFFFF;
}

static void pic_remap(void) {
    outb(0x20, 0x11);  io_wait();
    outb(0xA0, 0x11);  io_wait();
    outb(0x21, 0x20);  io_wait();
    outb(0xA1, 0x28);  io_wait();
    outb(0x21, 0x04);  io_wait();
    outb(0xA1, 0x02);  io_wait();
    outb(0x21, 0x01);  io_wait();
    outb(0xA1, 0x01);  io_wait();

    outb(0x21, 0xFC);  /* IRQ0 + IRQ1 only */
    outb(0xA1, 0xFF);
}

void idt_install(void) {
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    idtp.limit = sizeof(idt_entry_t) * 256 - 1;
    idtp.base  = (uint32_t)&idt;

    pic_remap();

    /* Configura PIT a 100 Hz (10ms por tick) */
    timer_init(100);

    /* Exceções */
    idt_set_gate(0, (uint32_t)exception_div_zero, 0x08, 0x8E);

    /* IRQs */
    idt_set_gate(0x20, (uint32_t)irq_handler_timer,    0x08, 0x8E);
    idt_set_gate(0x21, (uint32_t)irq_handler_keyboard, 0x08, 0x8E);

    idt_load((uint32_t)&idtp);

    vga_put_string("[IDT] PIC remapped. PIT @ 100Hz. Interrupts ON.\n", COLOR_GREEN);
}
