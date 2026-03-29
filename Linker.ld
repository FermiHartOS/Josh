/**
 * @file linker.ld
 * @brief Linker script para o Fermi Hart OS Pro.
 * Garante que o cabeçalho Multiboot esteja nos primeiros 8KB do binário.
 */

ENTRY(_start)

/* Define os cabeçalhos de programa para permissões de memória (Segurança) */
PHDRS
{
    text PT_LOAD FLAGS(5); /* Read (4) + Execute (1) */
    data PT_LOAD FLAGS(6); /* Read (4) + Write (2) */
}

SECTIONS
{
    /* O kernel começa em 1MB (convencional para kernels x86) */
    . = 1M;

    /* SEÇÃO DE CÓDIGO */
    .text BLOCK(4K) : ALIGN(4K)
    {
        /* O cabeçalho Multiboot PRECISA estar no topo para o QEMU/GRUB encontrar */
        KEEP(*(.multiboot))
        *(.text)
    } :text

    /* SEÇÃO DE DADOS APENAS LEITURA (Constantes) */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    } :data

    /* SEÇÃO DE DADOS (Variáveis inicializadas) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    } :data

    /* SEÇÃO BSS (Variáveis globais não inicializadas e Stack) */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    } :data

    /* Descarte de seções desnecessárias que podem poluir o binário */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
