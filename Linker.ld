/**
 * @file linker.ld
 * @brief Linker script para o jOSh OS.
 * Garante que o cabeçalho Multiboot esteja nos primeiros 8KB do binário.
 * Exporta _kernel_end para o PMM saber onde a memória livre começa.
 */

ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* Read + Execute */
    data PT_LOAD FLAGS(6); /* Read + Write */
}

SECTIONS
{
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot))
        *(.text)
    } :text

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    } :data

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    } :data

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    } :data

    /* Símbolo que marca o fim do kernel — PMM aloca a partir daqui */
    . = ALIGN(4K);
    _kernel_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
